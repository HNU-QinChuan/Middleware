
#include "tcp/Acceptor.hpp"
#include "tcp/Server.hpp"
#include<spdlog/spdlog.h>


namespace Hnu::Tcp {
  Acceptor::Acceptor(asio::io_context& ioc,const std::string& ip, unsigned port): 
  m_endpoint(asio::ip::make_address(ip), port), m_acceptor(ioc, m_endpoint) {

  }
  void Acceptor::run() {
    doAccept();
  }
  void Acceptor::doAccept() {
    auto server=std::make_shared<Server>(m_acceptor.get_executor().context());
    m_acceptor.async_accept(server->socket(),std::bind_front(&Acceptor::onAccept, this, server));
  }
  void Acceptor::onAccept(std::shared_ptr<Server> acceptServer, const boost::system::error_code& ec) {
    doAccept();
    if (ec) {
      spdlog::error("Accept Error: {}", ec.message());
      return;
    }
    acceptServer->run();
  }
}