#include "tcp/TcpInterface.hpp"


namespace Hnu::Tcp{
  TcpInterface::TcpInterface(const std::string& name,const std::string& type,int segment,const std::string& ip, int port)
    : Hnu::Interface::Interface(name, type, segment), m_ip(ip), m_port(port) {

  }
  void TcpInterface::run(){

  }
}