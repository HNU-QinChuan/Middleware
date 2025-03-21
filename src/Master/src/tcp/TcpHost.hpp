#pragma once

#include "interface/Host.hpp"

namespace Hnu::Tcp{
  class TcpHost:public Hnu::Interface::Host{
  public:
    TcpHost(const std::string& name, const std::string& type, int segment, const std::string& ip, int port);
  private:
    std::string m_ip;
    int m_port;
  };
}