#include "tcp/Server.hpp"
#include"interface/InterfaceRouter.hpp"
#include<spdlog/spdlog.h>


namespace Hnu::Tcp {
  Server::Server(asio::execution_context& ioc):m_stream(static_cast<asio::io_context&>(ioc)) {
  }

  beast::tcp_stream::socket_type& Server::socket() {
    return m_stream.socket();
  }

  void Server::run() {
    // m_stream.expires_after(std::chrono::seconds(30));
    doRead();
  }
  void Server::doRead() {
    m_buffer.clear();
    m_request.clear();
    m_request.body().clear();
    beast::http::async_read(m_stream, m_buffer, m_request, std::bind(&Server::onRead, shared_from_this(),std::placeholders::_1,std::placeholders::_2));
  }
  void Server::onRead(const boost::system::error_code& ec,std::size_t bytes) {
    if (ec) {
      spdlog::error("Read Error: {}", ec.message());
      m_stream.socket().close();
      return;
    }
    // spdlog::debug("Read data size {}", bytes);
    // spdlog::debug("Read data {}", m_request.body().size());
    Interface::InterfaceRouter::handle(m_request);

    doRead();
    // doWrite();
  }
  void Server::doWrite() {
    m_response.prepare_payload();
    beast::http::async_write(m_stream, m_response, std::bind(&Server::onWrite, shared_from_this(),std::placeholders::_1,std::placeholders::_2));
  }
  void Server::onWrite(const boost::system::error_code& ec,std::size_t bytes) {
    if (ec) {
      spdlog::error("Write Error: {}", ec.message());
      return;
    }
    doRead();
  }

}