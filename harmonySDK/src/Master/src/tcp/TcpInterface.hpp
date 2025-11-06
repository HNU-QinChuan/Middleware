#pragma once

#include "interface/Interface.hpp"
#include"tcp/Acceptor.hpp"


namespace Hnu::Tcp{
  class TcpInterface:public Hnu::Interface::Interface{
  public:
    TcpInterface(const std::string& name,const std::string& type,int segment,const std::string& ip, unsigned port);
    void run(asio::io_context& ioc) override;
  private:
    std::string m_ip;
    unsigned m_port;
    std::shared_ptr<Hnu::Tcp::Acceptor> m_acceptor;
    std::string m_type = "tcp";
  };
}