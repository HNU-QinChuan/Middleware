#include "interface/InterfaceRouter.hpp"
#include"InterfaceManager.hpp"
#include<spdlog/spdlog.h>



namespace Hnu::Interface {
  class NodeController {
  public:
    void handlePost(Request& req){
      std::string node = std::string(req["node"]);
      std::string host=req["src"].to_string();
      InterfaceManager::interfaceManager.hostlist[host]->addNode(node);
      spdlog::debug("Host {} add node {}", host, node);
    }
    void handleDelete(Request& req){
      std::string node = std::string(req["node"]);
      std::string host=req["src"].to_string();
      auto removeTopics=InterfaceManager::interfaceManager.hostlist[host]->deleteNode(node);
      for(auto& removeTopic:removeTopics){
        InterfaceManager::interfaceManager.topic2host[removeTopic].erase(host);
      }
    }
  };
  CONTROLLER_REGISTER(NodeController,"/node", 
    http::verb::post, &NodeController::handlePost,
    http::verb::delete_, &NodeController::handleDelete)
}