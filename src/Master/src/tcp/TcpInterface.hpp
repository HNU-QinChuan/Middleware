#pragma once

#include "interface/Interface.hpp"


namespace Hnu::Tcp{
  class TcpInterface:public Hnu::Interface::Interface{
  public:
    TcpInterface(const std::string& name,const std::string& type,int segment,const std::string& ip, int port);
  private:
    std::string m_ip;
    int m_port;
  };
}