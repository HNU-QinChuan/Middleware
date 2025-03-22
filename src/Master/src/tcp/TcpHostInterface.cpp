#include "tcp/TcpHostInterface.hpp"
#include <spdlog/spdlog.h>


namespace Hnu::Tcp{
  TcpHostInterface::TcpHostInterface(const std::string& name, const std::string& hName,const std::string& type, int segment, const std::string& ip, unsigned port)
    : Hnu::Interface::HostInterface(name, hName, segment,type), m_ip(ip), m_port(port) ,
      m_endpoint(asio::ip::make_address(ip), port) {
  }
  std::shared_ptr<TcpHostInterface> TcpHostInterface::shared_from_this() {
    return std::static_pointer_cast<TcpHostInterface>(Hnu::Interface::HostInterface::shared_from_this());
  }
  void TcpHostInterface::run(asio::io_context& ioc) {
    m_stream = std::make_unique<beast::tcp_stream>(ioc);
    // m_stream->expires_after(std::chrono::seconds(30));
    doConnect();
  }
  void TcpHostInterface::doConnect() {
    m_stream->async_connect(m_endpoint, std::bind_front(&TcpHostInterface::onConnect, shared_from_this()));
  }
  void TcpHostInterface::onConnect(beast::error_code ec) {
    if (ec) {
      spdlog::warn("Connect to {} error: {} ,and will reconnect next time",hostName, ec.message());
      return;
    }
    isConnected = true;
    // spdlog::info("Connected to {}:{}", m_ip, m_port);
  }
}