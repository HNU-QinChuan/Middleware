#include "tcp/TcpHost.hpp"


namespace Hnu::Tcp{
  TcpHost::TcpHost(const std::string& name, const std::string& type, int segment, const std::string& ip, int port)
    : Hnu::Interface::Host(name, type, segment), m_ip(ip), m_port(port) {
  }
}