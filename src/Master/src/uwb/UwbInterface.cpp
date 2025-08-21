#include "uwb/UwbInterface.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Uwb {

// 新构造函数（带node_id）
UwbInterface::UwbInterface(const std::string& name, const std::string& type, int segment,
                           const std::string& device, unsigned baudrate, uint8_t node_id)
    : Hnu::Interface::Interface(name, type, segment), m_device(device), m_baudrate(baudrate), m_node_id(node_id) {
    
    spdlog::info("UwbInterface created: name={}, device={}, baudrate={}, node_id={}", 
                name, device, baudrate, static_cast<int>(node_id));
}

// 原构造函数（向后兼容）
UwbInterface::UwbInterface(const std::string& name, const std::string& type, int segment,
                           const std::string& device, unsigned baudrate)
    : UwbInterface(name, type, segment, device, baudrate, 1) { // 默认node_id=1
}

void UwbInterface::run(asio::io_context& ioc) {
    if (m_handle) {
        spdlog::warn("UwbInterface::run() called more than once, ignoring.");
        return;
    }
    
    // 使用新的Handle构造函数（包含node_id）
    m_handle = std::make_shared<Hnu::Uwb::Handle>(ioc, m_device, m_baudrate, m_node_id);
    m_handle->run();
    
    spdlog::info("UwbInterface started for node {}", static_cast<int>(m_node_id));
}

void UwbInterface::broadcast(beast::http::request<beast::http::string_body>& req) {
    // 尝试从请求头中获取目标节点
    uint8_t broadcast_node = FrameCodec::BROADCAST_ID; // 默认广播
    
    if (req.count("target_node")) {
        try {
            broadcast_node = static_cast<uint8_t>(std::stoi(req.at("target_node").to_string()));
        } catch (const std::exception& e) {
            spdlog::warn("Invalid target_node in request, using broadcast: {}", e.what());
        }
    }

    m_handle->send_http_reliable(req, broadcast_node, nullptr);
}

std::shared_ptr<Hnu::Uwb::Handle> UwbInterface::getHandle() const {
    return m_handle;
}

Handle::Stats UwbInterface::getStats() const {
    if (m_handle) {
        return m_handle->get_stats();
    }
    return Handle::Stats{};
}

} // namespace Hnu::Uwb