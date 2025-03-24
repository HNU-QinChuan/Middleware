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
  };
  CONTROLLER_REGISTER(NodeController, "/node", http::verb::post, &NodeController::handlePost);
}