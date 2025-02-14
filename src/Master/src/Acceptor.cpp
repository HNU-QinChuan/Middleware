//
// Created by yc on 25-2-14.
//

#include "Acceptor.hpp"
#include "Server.hpp"
#include<spdlog/spdlog.h>
#include<filesystem>


namespace Hnu::Middleware {
  Acceptor::Acceptor():m_acceptor(m_ioc),m_endpoint("/tmp/master") {
    spdlog::debug("Acceptor Created");
  }
  void Acceptor::run() {
    spdlog::info("Doing Accept on {}", m_endpoint.path());
    std::filesystem::remove("/tmp/master");
    m_acceptor.open(asio::local::stream_protocol());
    m_acceptor.bind(m_endpoint);
    m_acceptor.listen(asio::socket_base::max_listen_connections);
    doAccept();
    m_ioc.run();
  }
  void Acceptor::doAccept() {
    auto server=std::make_shared<Server>(m_ioc);
    m_acceptor.async_accept(server->socket(),std::bind_front(&Acceptor::onAccept, this, server));
  }
  void Acceptor::onAccept(std::shared_ptr<Server> acceptServer, const boost::system::error_code& ec) {
    auto server=std::make_shared<Server>(m_ioc);
    m_acceptor.async_accept(server->socket(),std::bind_front(&Acceptor::onAccept, this, server));
    if (ec) {
      spdlog::error("Accept Error: {}", ec.message());
      return;
    }
    acceptServer->run();
  }




}