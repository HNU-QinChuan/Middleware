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
    doConnect();
  }
  void TcpHostInterface::doConnect() {
    if(isConnecting){
      return;
    }
    isConnected = false;
    isConnecting=true;
    m_stream->async_connect(m_endpoint, std::bind_front(&TcpHostInterface::onConnect, shared_from_this()));
  }
  void TcpHostInterface::onConnect(const beast::error_code& ec) {
    isConnecting=false;
    if (ec) {
      m_stream->close();
      spdlog::warn("Connect to {} error: {} errorcode: {},and will reconnect next time",hostName, ec.message(),ec.value());
      return;
    }
    isConnected = true;
    doWrite();
    onNew();
  }
  void TcpHostInterface::doWrite() {
    if(isReady&&isConnected&&!m_requestQueue.empty()){
      isReady=false;
      request=m_requestQueue.front();
      m_requestQueue.pop();
      beast::http::async_write(*m_stream, request,
        std::bind_front(&TcpHostInterface::onWrite, shared_from_this()));
    }
  }
  void TcpHostInterface::send(boost::beast::http::request<boost::beast::http::string_body>& req) {
    req.prepare_payload();
    m_requestQueue.push(req);
    if(!isConnected){
      doConnect();
      return;
    }
    doWrite();
  }
  void TcpHostInterface::onWrite(const beast::error_code& ec, std::size_t bytes_transferred) {
    isReady=true;
    if (ec) {
      spdlog::warn("Write to {} error: {}", hostName, ec.message());
      m_stream->close();
      doConnect();
      return;
    }
    doWrite();
    // doRead();
  }
  // void TcpHostInterface::doRead() {
  //   buffer.clear();
  //   response.clear();
  //   beast::http::async_read(*m_stream, buffer, response,
  //     std::bind_front(&TcpHostInterface::onRead, shared_from_this()));
  // }
  // void TcpHostInterface::onRead(const beast::error_code& ec, std::size_t bytes_transferred) {
  //   if (ec) {
  //     spdlog::warn("Read from {} error: {}", hostName, ec.message());
  //     m_stream->close();
  //     doConnect();
  //     return;
  //   }
  //   if (hasCallback) {
  //     callback(response);
  //     hasCallback = false;
  //   }
  //   isReady = true;
  // }

}