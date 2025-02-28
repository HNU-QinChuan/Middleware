//
// Created by yc on 25-2-14.
//

#include "MiddlewareManager.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  MiddlewareManager MiddlewareManager::middlewareManager;
  // MiddlewareManager& MiddlewareManager::getInstance() {
  //   // static MiddlewareManager middlewareManager;
  //   return middlewareManager;
  // }
  asio::io_context& MiddlewareManager::getIoc() {
    return middlewareManager.m_ioc;
  }
  void MiddlewareManager::run() {
    middlewareManager.m_ioc.run();
  }
  bool MiddlewareManager::addNode(const std::string& node,int pid) {
    if (middlewareManager.m_nodes.contains(node)) {
      return false;
    }
    middlewareManager.m_nodes[node]=std::make_shared<Node>(node,pid);
    spdlog::debug("Add Node: {}",node);
    return true;
  }
  bool MiddlewareManager::addPublish(const std::string& node,const std::string& topic,int eventfd){
    // MiddlewareManager &middlewareManager = getInstance();
    if(!middlewareManager.m_nodes.contains(node)){
      return false;
    }
    if(middlewareManager.m_nodes[node]->containsPublish(topic)){
      return false;
    }
    auto publish=std::make_shared<Publish>(topic,eventfd,middlewareManager.m_nodes[node]);
    if(!publish->run()){
      return false;
    }
    middlewareManager.m_nodes[node]->addPublish(publish);
    middlewareManager.m_publishes[topic].push_back(publish);
    spdlog::debug("Add Publish: {} {}",node,topic);
    return true;
  }
  bool MiddlewareManager::addSubscrie(const std::string& node, const std::string& topic, int eventfd) {
    // MiddlewareManager& middlewareManager = getInstance();
    if (!middlewareManager.m_nodes.contains(node)) {
      return false;
    }
    if (middlewareManager.m_nodes[node]->containsSubscribe(topic)) {
      return false;
    }
    auto subscribe = std::make_shared<Subscribe>(topic, eventfd, middlewareManager.m_nodes[node]);
    if (!subscribe->run()) {
      return false;
    }
    middlewareManager.m_nodes[node]->addSubscribe(subscribe);
    middlewareManager.m_subscribes[topic].push_back(subscribe);
    spdlog::debug("Add Subscribe: {} {}", node, topic);
    return true;
  }
  void MiddlewareManager::transferMessage(const std::string& topic, const std::string& message) {
    auto iter=middlewareManager.m_subscribes.find(topic);
    if(iter==middlewareManager.m_subscribes.end()){
      return;
    }
    for (auto subscribe:iter->second) {
      subscribe->publish2Node(message);
    }
  }

}
