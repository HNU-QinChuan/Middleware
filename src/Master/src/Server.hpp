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
    explicit Server();
    void run();
    local_stream::socket_type& socket();
  private:
    void doRead();
    void onRead(const boost::system::error_code& ec,std::size_t bytes);
    void doWrite();
    void onWrite(const boost::system::error_code& ec,std::size_t bytes);
    void handleRequest();
    void handleCreateNode();
    void handleCreatePublish();
    void handleCreateSubscribe();

    asio::io_context& m_ioc;
    local_stream m_stream;
    beast::http::request<beast::http::empty_body> m_request;
    beast::http::response<beast::http::empty_body> m_response;
    beast::flat_buffer m_buffer;

  };

} // Middleware
// Hnu


