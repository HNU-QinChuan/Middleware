#pragma once
#include<memory>
#include<boost/asio.hpp>
#include<boost/beast.hpp>

namespace Hnu::Uwb {
  namespace asio=boost::asio;
  namespace beast=boost::beast;
  class Handle :public std::enable_shared_from_this<Handle>{
  public:
    Handle(asio::io_context& ioc,const std::string& device, unsigned baudrate);
    void run();
    asio::serial_port m_serial;   // 串口对象

  private:
    void doRead();
    void doWrite();
    void onWrite(const boost::system::error_code& ec,std::size_t bytes);
  
  private:
    
    std::string m_device;         // 串口路径
    unsigned m_baudrate;          // 波特率
    beast::http::request<beast::http::string_body> m_request;
    beast::http::response<beast::http::string_body> m_response;
    beast::flat_buffer m_buffer;
  };
}