//
// Created by yc on 25-2-14.
//

#include "MiddlewareManager.hpp"

namespace Hnu::Middleware {
  MiddlewareManager& MiddlewareManager::getInstance() {
    static MiddlewareManager middlewareManager;
    return middlewareManager;
  }
  boost::asio::io_context& MiddlewareManager::getIoc() {
    return getInstance().m_ioc;
  }
  void MiddlewareManager::run() {
    getInstance().m_ioc.run();
  }
  bool MiddlewareManager::addNode(const std::string& node,int pid) {
    if (getInstance().m_nodes.contains(node)) {
      return false;
    }
    getInstance().m_nodes[node]=std::make_shared<Node>(node,pid);
    return true;
  }
  bool MiddlewareManager::addPublish(const std::string& node,const std::string& topic,int eventfd){
    MiddlewareManager &middlewareManager = getInstance();
    if(!middlewareManager.m_nodes.contains(node)){
      return false;
    }
    if(middlewareManager.m_nodes[node]->containsPublish(topic)){
      return false;
    }
    auto publish=std::make_shared<Publish>(topic,eventfd);
    middlewareManager.m_nodes[node]->addPublish(publish);
    if(!publish->run()){
      //TODO: middlewareManager.m_nodes[node]->removePublish(topic);
      return false;
    }

    middlewareManager.m_publishes[topic].push_back(publish);
    
    return true;
  }
}
