#pragma once

#include "interface/HostInterface.hpp"

namespace Hnu::Tcp{
  class TcpHostInterface:public Hnu::Interface::HostInterface{
  public:
  TcpHostInterface(const std::string& name, const std::string& type, int segment, const std::string& ip, int port);
  private:
    std::string m_ip;
    int m_port;
  };
}