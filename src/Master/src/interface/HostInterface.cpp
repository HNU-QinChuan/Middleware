#include "interface/HostInterface.hpp"
#include"MiddlewareManager.hpp"
#include "InterfaceManager.hpp"


namespace Hnu::Interface {
  HostInterface::HostInterface(const std::string& name,const std::string& hName, int segment, const std::string& type)
    : m_name(name), m_segment(segment), m_type(type),hostName(hName) {}
  // int HostInterface::getSegment() {
  //   return m_segment;
  // }

  // void HostInterface::send(http::request<http::string_body>& req,std::function<void(http::response<http::string_body>&)>& cb) {
  //   hasCallback = true;
  //   callback = cb;
  //   send(req);
  // }
  void HostInterface::onNew() {
    http::request<http::string_body> req{
      http::verb::post,
      "/new",
      11
    };
    req.body()=Middleware::MiddlewareManager::getAllNodeInfo();
    req.prepare_payload();
    InterfaceManager::broadcast(req);
  }
}