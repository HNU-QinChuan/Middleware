//
// Created by yc on 25-2-14.
//

#include "Server.hpp"
#include"MiddlewareManager.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  Server::Server():m_ioc(MiddlewareManager::getIoc()),m_stream(m_ioc) {
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
    pid_t pid = std::stoi(std::string(m_request["pid"]));
    std::string name = std::string(m_request["node"]);
    if (MiddlewareManager::addNode(name, pid)) {
      m_response.result(beast::http::status::ok);
    } else {
      m_response.result(beast::http::status::bad_request);
    }
  }
  void Server::handleCreatePublish() {
    int eventfd = std::stoi(std::string(m_request["eventfd"]));
    std::string topic = std::string(m_request["pub"]);
    std::string node = std::string(m_request["node"]);
    if (MiddlewareManager::addPublish(node, topic, eventfd)) {
      m_response.result(beast::http::status::ok);
    } else {
      m_response.result(beast::http::status::bad_request);
    }
  }
  void Server::handleCreateSubscribe() {
  }




}
