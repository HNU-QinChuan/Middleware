#pragma once
#include <memory>
#include<boost/asio.hpp>
#include<boost/beast.hpp>


namespace Hnu::Tcp {
  namespace asio=boost::asio;
  namespace beast=boost::beast;
  class Server :public std::enable_shared_from_this<Server> {
  public:
    Server(asio::execution_context& ioc);
    void run();
    beast::tcp_stream::socket_type& socket();
  private:
    void doRead();
    void onRead(const boost::system::error_code& ec,std::size_t bytes);
    void doWrite();
    void onWrite(const boost::system::error_code& ec,std::size_t bytes);

    beast::tcp_stream m_stream;
    beast::http::request<beast::http::string_body> m_request;
    beast::http::response<beast::http::string_body> m_response;
    beast::flat_buffer m_buffer;

  };

} 