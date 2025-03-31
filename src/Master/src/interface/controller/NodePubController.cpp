#include "InterfaceManager.hpp"
#include"interface/InterfaceRouter.hpp"
#include<spdlog/spdlog.h>




namespace Hnu::Interface {
  class NodePubController{
  public:
    void handlePost(Request& req){
      std::string node = std::string(req["node"]);
      std::string pub = std::string(req["pub"]);
      std::string type = std::string(req["type"]);
      std::string host=req["src"].to_string();
      InterfaceManager::interfaceManager.hostlist[host]->addPub(node, pub, type);
      spdlog::debug("Host {} add pub {} to node {} and type is {}", host, pub, node, type);
    }
  };
  CONTROLLER_REGISTER(NodePubController, "/node/pub", http::verb::post, &NodePubController::handlePost)
}