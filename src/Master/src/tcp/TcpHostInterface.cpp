#include "tcp/TcpHostInterface.hpp"


namespace Hnu::Tcp{
  TcpHostInterface::TcpHostInterface(const std::string& name, const std::string& type, int segment, const std::string& ip, int port)
    : Hnu::Interface::HostInterface(name,  segment,type), m_ip(ip), m_port(port) {
  }
}