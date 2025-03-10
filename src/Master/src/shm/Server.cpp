//
// Created by yc on 25-2-14.
//

#include "shm/Server.hpp"
#include <spdlog/spdlog.h>
#include "shm/UdsRouter.hpp"

namespace Hnu::Middleware {
  Server::Server(asio::execution_context& ioc):m_stream(static_cast<asio::io_context&>(ioc)) {
  }

  local_stream::socket_type& Server::socket() {
    return m_stream.socket();
  }

  void Server::run() {
    // m_stream.expires_after(std::chrono::seconds(30));
    doRead();
  }
  void Server::doRead() {
    // m_buffer.clear();
    // m_request.clear();
    beast::http::async_read(m_stream, m_buffer, m_request, std::bind_front(&Server::onRead, shared_from_this()));
  }
  void Server::onRead(const boost::system::error_code& ec,std::size_t bytes) {
    if (ec) {
      spdlog::error("Read Error: {}", ec.message());
      return;
    }
    UdsRouter::handle(m_request,m_response);
    doWrite();
  }
  void Server::doWrite() {
    m_response.prepare_payload();
    beast::http::async_write(m_stream, m_response, std::bind_front(&Server::onWrite, shared_from_this()));
  }
  void Server::onWrite(const boost::system::error_code& ec,std::size_t bytes) {
    if (ec) {
      spdlog::error("Write Error: {}", ec.message());
      return;
    }
  }




}
