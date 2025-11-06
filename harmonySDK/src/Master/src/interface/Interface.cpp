#include "interface/Interface.hpp"
#include<spdlog/spdlog.h>


namespace Hnu::Interface {
  Interface::Interface(const std::string& name, const std::string& type, int segment)
    : m_name(name), m_type(type), m_segment(segment) {}



  int Interface::getSegment()  { return m_segment; }

  void Interface::setHostInterface(const std::string& name, std::shared_ptr<HostInterface> hostInterface) {
    name2hostInterface[name] = hostInterface;
  }

  void Interface::start(asio::io_context& ioc) {
    run(ioc);
    for(auto&[key,value]:name2hostInterface){
      value->run(ioc);
    }

  }
  void Interface::send(const std::string& nextInterface,http::request<http::string_body>& req){
    // spdlog::debug("Send to {}", nextInterface);
    name2hostInterface[nextInterface]->send(req); 
  }
  // void Interface::send(std::string nextInterface,http::request<http::string_body>& req,std::function<void(http::response<http::string_body>&)>& cb){
  //   name2hostInterface[nextInterface]->send(req,cb); 
  // }
}