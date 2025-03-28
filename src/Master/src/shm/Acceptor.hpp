//
// Created by yc on 25-2-14.
//

#pragma once

#include<boost/asio.hpp>

#include "shm/Server.hpp"

namespace Hnu::Middleware {
  namespace asio=boost::asio;
  class Acceptor {
  public:
    Acceptor(asio::io_context& ioc);
    void run();
  private:
    void doAccept();
    void onAccept(std::shared_ptr<Server> acceptServer, const boost::system::error_code& ec);

    asio::local::stream_protocol::acceptor m_acceptor;
    asio::local::stream_protocol::endpoint m_endpoint;
  };




}



