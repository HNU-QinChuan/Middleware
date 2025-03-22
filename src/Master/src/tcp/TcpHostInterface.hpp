#pragma once

#include "interface/HostInterface.hpp"
#include <boost/asio.hpp>
#include<boost/beast.hpp>

namespace Hnu::Tcp{
  namespace asio = boost::asio;
  namespace beast = boost::beast;
  class TcpHostInterface:public Hnu::Interface::HostInterface{
  public:
  TcpHostInterface(const std::string& name,const std::string& hName, const std::string& type, int segment, const std::string& ip, unsigned port);
  void run(boost::asio::io_context& ioc) override;
  void doConnect();
  protected:
    std::shared_ptr<TcpHostInterface> shared_from_this();
  private:
    void onConnect(beast::error_code ec);


    std::string m_ip;
    unsigned m_port;
    asio::ip::tcp::endpoint m_endpoint;
    std::unique_ptr<beast::tcp_stream> m_stream;
    bool isConnected = false;
  };
}