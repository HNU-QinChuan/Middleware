#pragma once
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace Hnu::Interface {
  namespace http = boost::beast::http;
  class HostInterface : public std::enable_shared_from_this<HostInterface> {
  public:
    HostInterface(const std::string& name,const std::string& hName, int segment, const std::string& type);
    virtual ~HostInterface() = default;
    // int getSegment() ;
    virtual void run(boost::asio::io_context& ioc)=0;
    virtual void send(http::request<http::string_body>& req) = 0;
    void onNew();
    // virtual void send(http::request<http::string_body>& req,std::function<void(http::response<http::string_body>&)>& cb);

  protected:
    std::string m_name;
    std::string hostName;
    int m_segment;
    std::string m_type;
    // bool hasCallback = false;
    // http::response<http::string_body> response;
    http::request<http::string_body> request;
    // std::function<void(http::response<http::string_body>&)> callback;
  };
}