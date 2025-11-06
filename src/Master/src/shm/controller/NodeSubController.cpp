//
// Created by yc on 25-2-28.
//


#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"

namespace Hnu::Middleware {
  class NodeSubController  {
  public:
    void handlePost(Request& req, Response& res)  {
      int eventfd = std::stoi(std::string(req["eventfd"]));
      std::string topic = std::string(req["sub"]);
      std::string node = std::string(req["node"]);
      std::string type = std::string(req["type"]);
      if (addSubscrie(node, topic, eventfd,type)) {
        res.result(http::status::ok);
      } else {
        res.result(http::status::bad_request);
      }
    }
    bool addSubscrie(const std::string& node, const std::string& topic, int eventfd,const std::string& type) {
      // MiddlewareManager& middlewareManager = getInstance();
      if (MiddlewareManager::middlewareManager.m_nodes.find(node) == MiddlewareManager::middlewareManager.m_nodes.end()) {
        return false;
      }
      if (MiddlewareManager::middlewareManager.m_nodes[node]->containsSubscribe(topic)) {
        return false;
      }
      auto subscribe = std::make_shared<Subscribe>(topic, eventfd, MiddlewareManager::middlewareManager.m_nodes[node],type);
      if (!subscribe->run()) {
        return false;
      }
      MiddlewareManager::middlewareManager.m_nodes[node]->addSubscribe(subscribe);
      MiddlewareManager::middlewareManager.m_subscribes[topic].push_back(subscribe);
      Interface::InterfaceManager::addSub(node, topic,type);
      spdlog::debug("Add Subscribe: {} {}", node, topic);
      return true;
    }
  };
  CONTROLLER_REGISTER(NodeSubController, "/node/sub", http::verb::post,&NodeSubController::handlePost);
} // Middleware