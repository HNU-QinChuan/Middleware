#include "uwb/UwbHandle.hpp"
#include <spdlog/spdlog.h>
#include <sstream>

namespace Hnu::Uwb {

Handle::Handle(asio::io_context& ioc, const std::string& device, unsigned baudrate, uint8_t node_id)
    : m_ioc(ioc), m_device(device), m_baudrate(baudrate), m_node_id(node_id), m_serial(ioc) {
    
    m_slot_manager = std::make_unique<SimpleTimeSlotManager>(node_id);
    spdlog::info("UWB Handle created for node {} on device {}", static_cast<int>(node_id), device);
}

Handle::~Handle() {
    stop();
}

void Handle::run() {
    if (m_running.load()) return;
    
    if (!init_serial()) {
        spdlog::error("Failed to initialize serial port for node {}", static_cast<int>(m_node_id));
        return;
    }
    
    m_running = true;
    
    // 设置时隙回调
    m_slot_manager->set_my_slot_callback([this]() { on_my_slot(); });
    
    // 启动时隙管理器
    m_slot_manager->start();
    
    // 启动异步读取
    start_async_read();
    
    // 启动后台线程
    m_timeout_thread = std::thread(&Handle::timeout_worker, this);
    m_heartbeat_thread = std::thread(&Handle::heartbeat_worker, this);
    
    spdlog::info("UWB Handle started for node {}", static_cast<int>(m_node_id));
}

void Handle::stop() {
    if (!m_running.load()) return;
    
    m_running = false;
    
    m_slot_manager->stop();
    
    if (m_serial.is_open()) {
        m_serial.close();
    }
    
    // 等待后台线程结束
    if (m_timeout_thread.joinable()) {
        m_timeout_thread.join();
    }
    if (m_heartbeat_thread.joinable()) {
        m_heartbeat_thread.join();
    }
    
    // 通知所有等待的回调失败
    {
        std::lock_guard<std::mutex> lock(m_pending_mutex);
        for (auto& pair : m_pending_acks) {
            if (pair.second.callback) {
                pair.second.callback(false);
            }
        }
        m_pending_acks.clear();
    }
    
    m_send_cv.notify_all();
    
    spdlog::info("UWB Handle stopped for node {}", static_cast<int>(m_node_id));
}

void Handle::send_http_reliable(const http::request<http::string_body>& req, 
                               uint8_t dst_id, 
                               std::function<void(bool)> callback) {
    if (!m_running.load()) {
        if (callback) callback(false);
        return;
    }
    
    std::string serialized = serialize_http_request(req);
    uint8_t seq = m_next_seq++;
    if (m_next_seq == 0) m_next_seq = 1; // 避免序列号0
    
    TimeSlotFrame frame(m_node_id, dst_id, seq, FrameType::DATA, serialized);
    
    // 添加到发送队列
    {
        std::lock_guard<std::mutex> lock(m_send_mutex);
        m_send_queue.push(frame);
    }
    
    // 如果需要ACK确认，添加到等待列表
    if (dst_id != FrameCodec::BROADCAST_ID) {
        std::lock_guard<std::mutex> lock(m_pending_mutex);
        m_pending_acks.emplace(seq, PendingFrame(frame, callback));
    } else {
        // 广播不需要ACK，直接成功回调
        if (callback) callback(true);
    }
    
    {
        std::lock_guard<std::mutex> lock(m_stats_mutex);
        m_stats.http_requests_sent++;
    }
    
    m_send_cv.notify_one();
    
    spdlog::debug("HTTP request queued reliably to node {}, seq {}", 
                 static_cast<int>(dst_id), static_cast<int>(seq));
}

void Handle::send_http(const http::request<http::string_body>& req, uint8_t dst_id) {
    send_http_reliable(req, dst_id, nullptr); // 无回调的可靠发送
}

Handle::Stats Handle::get_stats() const {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    return m_stats;
}

bool Handle::init_serial() {
    try {
        boost::system::error_code ec;
        m_serial.open(m_device, ec);
        if (ec) {
            spdlog::error("Failed to open serial device {}: {}", m_device, ec.message());
            return false;
        }
        
        m_serial.set_option(asio::serial_port_base::baud_rate(m_baudrate));
        m_serial.set_option(asio::serial_port_base::character_size(8));
        m_serial.set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
        m_serial.set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
        m_serial.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));
        
        spdlog::info("Serial port {} initialized for node {}", m_device, static_cast<int>(m_node_id));
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Serial initialization error: {}", e.what());
        return false;
    }
}

void Handle::start_async_read() {
    auto self = shared_from_this();
    
    asio::async_read_until(m_serial, m_read_buffer, '\n',
        [this, self](const boost::system::error_code& ec, std::size_t bytes) {
            handle_read(ec, bytes);
        });
}

void Handle::handle_read(const boost::system::error_code& ec, std::size_t bytes) {
    if (!m_running.load()) return;
    
    if (!ec) {
        std::istream is(&m_read_buffer);
        std::string line;
        std::getline(is, line);
        
        // 移除回车换行
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // 解码并处理帧
        TimeSlotFrame frame;
        if (FrameCodec::decode(line, frame)) {
            handle_received_frame(frame);
        } else {
            std::lock_guard<std::mutex> lock(m_stats_mutex);
            m_stats.serial_errors++;
            spdlog::debug("Failed to decode frame: {}", line);
        }
        
    } else {
        std::lock_guard<std::mutex> lock(m_stats_mutex);
        m_stats.serial_errors++;
        spdlog::error("Serial read error: {}", ec.message());
    }
    
    // 继续读取
    start_async_read();
}

void Handle::handle_received_frame(const TimeSlotFrame& frame) {
    // 过滤不是发给我的帧（除非是广播）
    if (frame.dst_id != m_node_id && frame.dst_id != FrameCodec::BROADCAST_ID) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(m_stats_mutex);
        m_stats.frames_received++;
    }
    
    spdlog::debug("Received frame from node {} to {}, seq {}, type {}", 
                 static_cast<int>(frame.src_id), static_cast<int>(frame.dst_id),
                 static_cast<int>(frame.seq_no), static_cast<int>(frame.type));
    
    if (frame.type == FrameType::ACK) {
        // 处理ACK
        std::lock_guard<std::mutex> lock(m_pending_mutex);
        auto it = m_pending_acks.find(frame.seq_no);
        if (it != m_pending_acks.end()) {
            if (it->second.callback) {
                it->second.callback(true); // 成功回调
            }
            m_pending_acks.erase(it);
            
            {
                std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
                m_stats.acks_received++;
            }
            
            spdlog::debug("ACK processed for seq {}", static_cast<int>(frame.seq_no));
        }
        
    } else if (frame.type == FrameType::DATA || frame.type == FrameType::HEARTBEAT) {
        // 检查重复帧
        bool is_duplicate = false;
        {
            std::lock_guard<std::mutex> lock(m_recv_mutex);
            auto it = m_last_received_seq.find(frame.src_id);
            if (it != m_last_received_seq.end() && it->second == frame.seq_no) {
                is_duplicate = true;
            } else {
                m_last_received_seq[frame.src_id] = frame.seq_no;
            }
        }
        
        if (is_duplicate) {
            spdlog::debug("Duplicate frame filtered from node {}, seq {}", 
                         static_cast<int>(frame.src_id), static_cast<int>(frame.seq_no));
            return;
        }
        
        // 发送ACK（如果不是广播且不是心跳）
        if (frame.dst_id != FrameCodec::BROADCAST_ID && frame.type != FrameType::HEARTBEAT) {
            TimeSlotFrame ack_frame(m_node_id, frame.src_id, frame.seq_no, FrameType::ACK);
            
            {
                std::lock_guard<std::mutex> lock(m_ack_mutex);
                m_ack_queue.push(ack_frame);
            }
        }
        
        // 处理数据
        if (frame.type == FrameType::DATA) {
            std::string payload(frame.payload.begin(), frame.payload.end());
            http::request<http::string_body> req;
            
            if (parse_http_request(payload, req)) {
                {
                    std::lock_guard<std::mutex> lock(m_stats_mutex);
                    m_stats.http_requests_received++;
                }
                
                Interface::InterfaceRouter::handle(req);
            } else {
                spdlog::warn("Failed to parse HTTP request from node {}", 
                            static_cast<int>(frame.src_id));
            }
        }
    }
}

void Handle::send_frame_immediately(const TimeSlotFrame& frame) {
    if (!m_serial.is_open()) return;
    
    std::string encoded = FrameCodec::encode(frame);
    encoded += "\n"; // 添加换行符
    
    try {
        asio::write(m_serial, asio::buffer(encoded));
        
        {
            std::lock_guard<std::mutex> lock(m_stats_mutex);
            m_stats.frames_sent++;
            if (frame.type == FrameType::ACK) {
                m_stats.acks_sent++;
            }
        }
        
        spdlog::debug("Frame sent to node {}, seq {}, type {}", 
                     static_cast<int>(frame.dst_id), static_cast<int>(frame.seq_no), 
                     static_cast<int>(frame.type));
        
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(m_stats_mutex);
        m_stats.serial_errors++;
        spdlog::error("Serial write error: {}", e.what());
    }
}

void Handle::on_my_slot() {
    // 发送所有排队的数据帧
    {
        std::lock_guard<std::mutex> lock(m_send_mutex);
        while (!m_send_queue.empty()) {
            send_frame_immediately(m_send_queue.front());
            m_send_queue.pop();
        }
    }
    
    // 发送所有排队的ACK帧
    {
        std::lock_guard<std::mutex> lock(m_ack_mutex);
        while (!m_ack_queue.empty()) {
            send_frame_immediately(m_ack_queue.front());
            m_ack_queue.pop();
        }
    }
}

void Handle::timeout_worker() {
    while (m_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100ms检查间隔
        if (m_running.load()) {
            check_timeouts();
        }
    }
}

void Handle::check_timeouts() {
    auto now = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(m_pending_mutex);
    
    for (auto it = m_pending_acks.begin(); it != m_pending_acks.end();) {
        auto& pending = it->second;
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - pending.send_time);
        
        if (elapsed.count() >= ACK_TIMEOUT_MS) {
            if (pending.retry_count < MAX_RETRIES) {
                // 重传
                pending.retry_count++;
                pending.send_time = now;
                
                {
                    std::lock_guard<std::mutex> send_lock(m_send_mutex);
                    m_send_queue.push(pending.frame);
                }
                
                {
                    std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
                    m_stats.retransmissions++;
                }
                
                spdlog::debug("Retransmitting seq {}, attempt {}/{}", 
                            static_cast<int>(pending.frame.seq_no), 
                            pending.retry_count, MAX_RETRIES);
                ++it;
            } else {
                // 超过最大重传次数，失败
                if (pending.callback) {
                    pending.callback(false);
                }
                
                {
                    std::lock_guard<std::mutex> stats_lock(m_stats_mutex);
                    m_stats.timeouts++;
                }
                
                spdlog::warn("Frame seq {} timed out after {} retries", 
                           static_cast<int>(pending.frame.seq_no), MAX_RETRIES);
                it = m_pending_acks.erase(it);
            }
        } else {
            ++it;
        }
    }
}

void Handle::heartbeat_worker() {
    while (m_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_INTERVAL_MS));
        
        if (m_running.load()) {
            std::string heartbeat_data = "HB:" + std::to_string(m_node_id);
            TimeSlotFrame hb_frame(m_node_id, FrameCodec::BROADCAST_ID, 0, FrameType::HEARTBEAT, heartbeat_data);
            
            {
                std::lock_guard<std::mutex> lock(m_send_mutex);
                m_send_queue.push(hb_frame);
            }
            
            {
                std::lock_guard<std::mutex> lock(m_stats_mutex);
                m_stats.heartbeats_sent++;
            }
            
            spdlog::debug("Heartbeat queued from node {}", static_cast<int>(m_node_id));
        }
    }
}

bool Handle::parse_http_request(const std::string& data, http::request<http::string_body>& req) {
    try {
        std::istringstream iss(data);
        std::string line;
        bool first_line = true;
        
        // 解析请求行和头部
        while (std::getline(iss, line) && !line.empty()) {
            if (line.back() == '\r') line.pop_back();
            
            if (first_line) {
                // 解析请求行
                std::istringstream line_stream(line);
                std::string method, path, version;
                line_stream >> method >> path >> version;
                
                // 设置HTTP方法
                if (method == "GET") req.method(http::verb::get);
                else if (method == "POST") req.method(http::verb::post);
                else if (method == "PUT") req.method(http::verb::put);
                else if (method == "DELETE") req.method(http::verb::delete_);
                else req.method(http::verb::unknown);
                
                req.target(path);
                req.version(11);
                first_line = false;
                
            } else {
                // 解析头部
                auto colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(colon_pos + 1);
                    
                    // 去掉前后空格
                    while (!value.empty() && value[0] == ' ') value.erase(0, 1);
                    while (!value.empty() && value.back() == ' ') value.pop_back();
                    
                    req.set(key, value);
                }
            }
        }
        
        // 读取body
        std::string body;
        std::string remaining_line;
        while (std::getline(iss, remaining_line)) {
            body += remaining_line + "\n";
        }
        if (!body.empty() && body.back() == '\n') body.pop_back();
        
        req.body() = body;
        req.prepare_payload();
        
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("HTTP parse error: {}", e.what());
        return false;
    }
}

std::string Handle::serialize_http_request(const http::request<http::string_body>& req) {
    std::ostringstream oss;
    oss << req;
    return oss.str();
}

void Handle::doRead() {
    // 保持与原有接口的兼容性
    // 实际读取在start_async_read()中处理
}

} // namespace Hnu::Uwb