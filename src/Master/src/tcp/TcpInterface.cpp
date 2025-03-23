#include "tcp/TcpInterface.hpp"


namespace Hnu::Tcp{
  TcpInterface::TcpInterface(const std::string& name,const std::string& type,int segment,const std::string& ip, unsigned port)
    : Hnu::Interface::Interface(name, type, segment), m_ip(ip), m_port(port) {

  }
  void TcpInterface::run(asio::io_context& ioc){
    m_acceptor = std::make_shared<Hnu::Tcp::Acceptor>(ioc, m_ip, m_port);
    m_acceptor->run();
  }
}