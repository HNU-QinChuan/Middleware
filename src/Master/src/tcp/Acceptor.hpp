#pragma once


#include<boost/asio.hpp>
#include "tcp/Server.hpp"

namespace Hnu::Tcp {
  namespace asio=boost::asio;
  class Acceptor {
  public:
    Acceptor(asio::io_context& ioc,const std::string& ip, unsigned port);
    void run();
  private:
    void doAccept();
    void onAccept(std::shared_ptr<Server> acceptServer, const boost::system::error_code& ec);

    asio::ip::tcp::endpoint m_endpoint;
    asio::ip::tcp::acceptor m_acceptor;

  };




}