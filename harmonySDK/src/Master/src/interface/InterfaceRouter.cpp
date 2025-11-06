#include "interface/InterfaceRouter.hpp"
#include "InterfaceManager.hpp"


namespace Hnu::Interface {
  InterfaceRouter InterfaceRouter::m_instance;
  InterfaceRouter& InterfaceRouter::getInstance() {
    // static UdsRouter instance;
    return m_instance;
  }
  void InterfaceRouter::registerController(const std::string &path, http::verb verb, std::function<void (Request &)> func){
    getInstance().m_router[path][verb]=func;
  }
  void InterfaceRouter::handle(Request &req) {
    std::string dest=req["dest"].to_string();
    if(dest!=InterfaceManager::interfaceManager.m_hostName){
      InterfaceManager::interfaceManager.transfer(dest,req);
      return;
    }
    auto& instance=getInstance();
    auto it=instance.m_router.find(req.target().to_string());
    if (it==instance.m_router.end()) {
      return;
    }
    auto it2=it->second.find(req.method());
    if (it2==it->second.end()) {
      return;
    }
    it2->second(req);
  }
} 