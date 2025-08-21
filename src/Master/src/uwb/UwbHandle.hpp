#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include "uwb/UwbFramecodec.hpp"
#include "uwb/UwbTimeslotmanager.hpp"
#include "interface/InterfaceRouter.hpp" 

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

namespace Hnu::Uwb {

class Handle : public std::enable_shared_from_this<Handle> {
public:
    // 构造函数：添加node_id参数
    Handle(asio::io_context& ioc, const std::string& device, unsigned baudrate, uint8_t node_id);
    virtual ~Handle();
    
    virtual void run();
    void stop();
    
    // 可靠发送（等待ACK确认）
    void send_http_reliable(const http::request<http::string_body>& req, 
                          uint8_t dst_id, 
                          std::function<void(bool)> callback = nullptr);
    
    // 简单发送（发送并忘记）
    void send_http(const http::request<http::string_body>& req, 
                   uint8_t dst_id = FrameCodec::BROADCAST_ID);
    
    
    // 统计信息
    struct Stats {
        uint32_t frames_sent = 0;
        uint32_t frames_received = 0;
        uint32_t acks_sent = 0;
        uint32_t acks_received = 0;
        uint32_t retransmissions = 0;
        uint32_t timeouts = 0;
        uint32_t http_requests_sent = 0;
        uint32_t http_requests_received = 0;
        uint32_t serial_errors = 0;
        uint32_t heartbeats_sent = 0;
    };
    Stats get_stats() const;
    
    uint8_t get_node_id() const { return m_node_id; }
    bool is_running() const { return m_running.load(); }
    
    asio::serial_port m_serial; // 保持公开访问

protected:
    virtual void doRead();

private:
    asio::io_context& m_ioc;
    std::string m_device;
    unsigned m_baudrate;
    uint8_t m_node_id;
    std::atomic<bool> m_running{false};
    
    // 时隙管理
    std::unique_ptr<SimpleTimeSlotManager> m_slot_manager;
    
    // 发送队列管理
    std::queue<TimeSlotFrame> m_send_queue;        // 待发送数据队列
    std::queue<TimeSlotFrame> m_ack_queue;         // 待发送ACK队列
    std::mutex m_send_mutex;
    std::mutex m_ack_mutex;
    std::condition_variable m_send_cv;
    
    // 可靠传输：等待ACK的帧
    struct PendingFrame {
        TimeSlotFrame frame;
        std::chrono::steady_clock::time_point send_time;
        uint32_t retry_count;
        std::function<void(bool)> callback;
        
        PendingFrame(const TimeSlotFrame& f, std::function<void(bool)> cb = nullptr)
            : frame(f), send_time(std::chrono::steady_clock::now()), retry_count(0), callback(cb) {}
    };
    std::unordered_map<uint8_t, PendingFrame> m_pending_acks; // seq_no -> pending_frame
    std::mutex m_pending_mutex;
    
    // 接收去重
    std::unordered_map<uint8_t, uint8_t> m_last_received_seq; // src_id -> last_seq
    std::mutex m_recv_mutex;
    uint8_t m_next_seq = 1;
    
    // 串口通信
    asio::streambuf m_read_buffer;
    
    
    // 后台线程
    std::thread m_timeout_thread;
    std::thread m_heartbeat_thread;
    
    // 统计
    mutable Stats m_stats;
    mutable std::mutex m_stats_mutex;
    
    // 常量
    static constexpr uint32_t ACK_TIMEOUT_MS = 2000;  // 2秒ACK超时
    static constexpr uint32_t MAX_RETRIES = 3;        // 最多重传3次
    static constexpr uint32_t HEARTBEAT_INTERVAL_MS = 5000; // 5秒心跳
    
    // 私有方法
    bool init_serial();
    void start_async_read();
    void handle_read(const boost::system::error_code& ec, std::size_t bytes);
    void handle_received_frame(const TimeSlotFrame& frame);
    void send_frame_immediately(const TimeSlotFrame& frame);
    
    // 时隙回调
    void on_my_slot();
    
    // 后台任务
    void timeout_worker();
    void heartbeat_worker();
    void check_timeouts();
    
    // HTTP解析
    bool parse_http_request(const std::string& data, http::request<http::string_body>& req);
    std::string serialize_http_request(const http::request<http::string_body>& req);
};

} // namespace Hnu::Uwb