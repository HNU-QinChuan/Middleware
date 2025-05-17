//
// Created by yc on 25-2-28.
//


#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include"InterfaceManager.hpp"

namespace Hnu::Middleware {
  class NodePubController  {
  public:
    void handlePost(Request& req, Response& res)  {
      int eventfd = std::stoi(std::string(req["eventfd"]));
      std::string topic = std::string(req["pub"]);
      std::string node = std::string(req["node"]);
      std::string type = std::string(req["type"]);
      if (addPublish(node, topic, eventfd,type)) {
        res.result(http::status::ok);
      } else {
        res.result(http::status::bad_request);
      }
    }
    bool addPublish(const std::string& node,const std::string& topic,int eventfd,const std::string& type) {
      // MiddlewareManager &middlewareManager = getInstance();
      if(MiddlewareManager::middlewareManager.m_nodes.find(node) == MiddlewareManager::middlewareManager.m_nodes.end()){
        return false;
      }
      if(MiddlewareManager::middlewareManager.m_nodes[node]->containsPublish(topic)){
        return false;
      }
      auto publish=std::make_shared<Publish>(topic,eventfd,MiddlewareManager::middlewareManager.m_nodes[node],type);
      if(!publish->run()){
        return false;
      }
      MiddlewareManager::middlewareManager.m_nodes[node]->addPublish(publish);
      MiddlewareManager::middlewareManager.m_publishes[topic].push_back(publish);
      Interface::InterfaceManager::addPub(node, topic,type);
      spdlog::debug("Add Publish: {} {}",node,topic);
      return true;
    }
  };
  CONTROLLER_REGISTER(NodePubController, "/node/pub", http::verb::post,&NodePubController::handlePost);
} // Middleware