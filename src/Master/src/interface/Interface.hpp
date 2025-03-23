#pragma once

#include <string>
#include<memory>
#include<boost/asio.hpp>
#include <unordered_map>
#include "interface/HostInterface.hpp"
#include<boost/beast.hpp>

namespace Hnu::Interface{
  namespace asio = boost::asio;
  namespace http = boost::beast::http;
  class Interface:public std::enable_shared_from_this<Interface>{
  public:
    Interface(const std::string& name, const std::string& type, int segment);
    virtual ~Interface() = default;
    int getSegment();
    void setHostInterface(const std::string& name, std::shared_ptr<HostInterface> hostInterface);
    void send(std::string nextInterface,http::request<http::string_body>& req);
    // void send(std::string nextInterface,http::request<http::string_body>& req,std::function<void(http::response<http::string_body>&)>& cb);

    virtual void run(asio::io_context& ioc) = 0;
    void start(asio::io_context& ioc);

  protected:
    std::string m_name;
    int m_segment;
    std::string m_type;

    std::unordered_map<std::string, std::shared_ptr<HostInterface>> name2hostInterface;

  };
}