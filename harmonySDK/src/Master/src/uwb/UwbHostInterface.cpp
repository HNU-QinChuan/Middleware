#include "uwb/UwbHostInterface.hpp"
#include "interface/InterfaceRouter.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Uwb {

UwbHostInterface::UwbHostInterface(const std::string& name, const std::string& hName, 
                                   const std::string& type, int segment, 
                                   const std::shared_ptr<Handle>& handle,
                                   uint8_t node_id)
    : Hnu::Interface::HostInterface(name, hName, segment, type), m_handle(handle), hNode_id(node_id) {
    
    spdlog::info("UwbHostInterface created: name={}, host={}", name, hName);
}

std::shared_ptr<UwbHostInterface> UwbHostInterface::shared_from_this() {
    return std::static_pointer_cast<UwbHostInterface>(Hnu::Interface::HostInterface::shared_from_this());
}

void UwbHostInterface::run(asio::io_context& ioc) {
    if (!m_handle) {
        spdlog::error("UwbHostInterface::run() called without valid handle");
        return;
    }
    
    // 🔥 关键修改：移除回调设置，Handle会直接调用InterfaceRouter
    // m_handle->set_http_receive_callback([this](const auto& req, uint8_t from_node) {
    //     on_http_received(req, from_node);
    // });
    
    spdlog::info("UwbHostInterface running for host {}", hostName);
}

void UwbHostInterface::send_reliable(beast::http::request<beast::http::string_body>& req,
                                    uint8_t target_node,
                                    std::function<void(bool)> callback) {
    if (!m_handle) {
        spdlog::error("UwbHostInterface::send_reliable() called without valid handle");
        if (callback) callback(false);
        return;
    }
    
    req.prepare_payload();
    m_handle->send_http_reliable(req, target_node, callback);

    spdlog::debug("HTTP request sent reliably to node {}", static_cast<int>(hNode_id));
}

void UwbHostInterface::send(beast::http::request<beast::http::string_body>& req) {
    send_reliable(req, hNode_id, nullptr);
}

// void UwbHostInterface::send_all(beast::http::request<beast::http::string_body>& req) {
//     // 尝试从请求头中获取目标节点
//     uint8_t broadcast_node = FrameCodec::BROADCAST_ID; // 默认广播
    
//     if (req.count("target_node")) {
//         try {
//             broadcast_node = static_cast<uint8_t>(std::stoi(req.at("target_node").to_string()));
//         } catch (const std::exception& e) {
//             spdlog::warn("Invalid target_node in request, using broadcast: {}", e.what());
//         }
//     }

//     send_reliable(req, broadcast_node, nullptr);
// }

void UwbHostInterface::doWrite() {
    // 保持与原有接口的兼容性
}

void UwbHostInterface::onWrite(const beast::error_code& ec, std::size_t bytes_transferred) {
    // 保持与原有接口的兼容性
}

// 🔥 这个函数现在不会被调用，但保留以备将来扩展
void UwbHostInterface::on_http_received(const beast::http::request<beast::http::string_body>& req, uint8_t from_node) {
    // 将接收到的HTTP请求路由到现有系统
    auto mutable_req = const_cast<beast::http::request<beast::http::string_body>&>(req);
    
    // 添加源节点信息
    mutable_req.set("src_node", std::to_string(from_node));
    mutable_req.set("dest", hostName);
    
    spdlog::debug("HTTP request received from node {}: {} {}", 
                static_cast<int>(from_node), 
                mutable_req.method_string().to_string(),
                mutable_req.target().to_string());
    
    // 路由到现有处理系统
    Hnu::Interface::InterfaceRouter::handle(mutable_req);
}

} // namespace Hnu::Uwb