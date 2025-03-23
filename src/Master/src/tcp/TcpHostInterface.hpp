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
  void run(asio::io_context& ioc) override;
  void doConnect();
  void send(beast::http::request<beast::http::string_body>& req) override;
  protected:
    std::shared_ptr<TcpHostInterface> shared_from_this();
  private:
    void onConnect(const beast::error_code& ec);
    void onWrite(const beast::error_code& ec, std::size_t bytes_transferred);
    // void doRead();
    // void onRead(const beast::error_code& ec, std::size_t bytes_transferred);


    std::string m_ip;
    unsigned m_port;
    asio::ip::tcp::endpoint m_endpoint;
    std::unique_ptr<beast::tcp_stream> m_stream;
    beast::flat_buffer buffer;
    bool isConnected = false;
    bool isReady = true;
  };
}