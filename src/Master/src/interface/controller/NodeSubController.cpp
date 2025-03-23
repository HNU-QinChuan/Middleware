#include "InterfaceManager.hpp"
#include"interface/InterfaceRouter.hpp"




namespace Hnu::Interface {
  class NodeSubController{
  public:
    void handlePost(Request& req){
      std::string node = std::string(req["node"]);
      std::string sub = std::string(req["sub"]);
      std::string type = std::string(req["type"]);
      std::string host=req["src"].to_string();
      InterfaceManager::interfaceManager.hostlist[host]->addSub(node,sub,type);
    }
  };
  CONTROLLER_REGISTER(NodeSubController, "/node/sub", http::verb::post, &NodeSubController::handlePost);
}