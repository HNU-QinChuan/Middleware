//
// Created by yc on 25-2-14.
//

#include "Server.hpp"

#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  Server::Server(asio::io_context& ioc):m_ioc(ioc),m_stream(ioc) {
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
    handleRequest();
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
  void Server::handleRequest() {
    beast::string_view target=m_request.target();
    //TODO: Add more routes
    //TODO: 使用策略模式优化
    if (target=="/node"&&m_request.method()==beast::http::verb::post) {
      handleCreateNode();
    }else if (target=="/node/pub"&&m_request.method()==beast::http::verb::post) {
      handleCreatePublish();
    }else if (target=="node/sub"&&m_request.method()==beast::http::verb::post) {
      handleCreateSubscribe();
    }
  }
  void Server::handleCreateNode() {
  }




}
