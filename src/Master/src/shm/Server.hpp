//
// Created by yc on 25-2-14.
//


#pragma once

#include <memory>
#include<boost/asio.hpp>
#include<boost/beast.hpp>


namespace Hnu::Middleware {
  namespace asio=boost::asio;
  namespace beast=boost::beast;
  using local_stream=beast::basic_stream<asio::local::stream_protocol>;
  class Server :public std::enable_shared_from_this<Server> {
  public:
    Server(asio::execution_context& ioc);
    void run();
    local_stream::socket_type& socket();
  private:
    void doRead();
    void onRead(const boost::system::error_code& ec,std::size_t bytes);
    void doWrite();
    void onWrite(const boost::system::error_code& ec,std::size_t bytes);

    local_stream m_stream;
    beast::http::request<beast::http::string_body> m_request;
    beast::http::response<beast::http::string_body> m_response;
    beast::flat_buffer m_buffer;

  };

} // Middleware
// Hnu


