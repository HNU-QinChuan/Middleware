//
// Created by yc on 25-2-14.
//

#include "shm/Node.hpp"

#include "MiddlewareManager.hpp"
#include "shm/Publish.hpp"
#include "shm/Subscribe.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  Node::Node(const std::string& name,int pid):m_name(name),m_pid(pid) {
  }
  Node::~Node() {
    // for (auto& [topic,publish]:m_publishes) {
    //   publish.reset();
    // }
    // for ()
    spdlog::debug("Destroy Node:{}",m_name);
  }

  std::string Node::getName() {
    return m_name;
  }
  pid_t Node::getPid() {
    return m_pid;
  }
  bool Node::containsPublish(const std::string& topic) {
    return m_publishes.find(topic) != m_publishes.end();
  }
  bool Node::containsSubscribe(const std::string& topic) {
    return m_subscribes.find(topic) != m_subscribes.end();
  }
  void Node::addPublish(std::shared_ptr<Publish> publish) {
    m_publishes[publish->getName()]=publish;
    // publish->setNode(shared_from_this());
  }
  void Node::addSubscribe(std::shared_ptr<Subscribe> subscribe) {
    m_subscribes[subscribe->getName()]=subscribe;
    // subscribe->setNode(shared_from_this());
  }
  std::shared_ptr<Publish> Node::removePublish(const std::string& topic) {
    auto iter=m_publishes.find(topic);
    if(iter==m_publishes.end()){
      return nullptr;
    }
    auto res=iter->second;
    m_publishes.erase(iter);
    return res;
  }
  std::shared_ptr<Subscribe> Node::removeSubscribe(const std::string& topic) {
    auto iter=m_subscribes.find(topic);
    if(iter==m_subscribes.end()){
      return nullptr;
    }
    auto res=iter->second;
    m_subscribes.erase(iter);
    return res;
  }
  std::vector<std::shared_ptr<Publish>> Node::removeAllPublish() {
    std::vector<std::shared_ptr<Publish>> res;
    for (auto& [topic,publish]:m_publishes) {
      res.push_back(publish);
    }
    m_publishes.clear();
    return res;
  }
  std::vector<std::shared_ptr<Subscribe>> Node::removeAllSubscribe() {
    std::vector<std::shared_ptr<Subscribe>> res;
    for (auto& [topic,subscribe]:m_subscribes) {
      res.push_back(subscribe);
    }
    m_subscribes.clear();
    return res;
  }
  std::vector<std::pair<std::string, std::string>> Node::getAllPublishTopics() {
    std::vector<std::pair<std::string, std::string>> res;
    for (auto& [topic,publish]:m_publishes) {
      res.push_back({topic,publish->getType()});
    }
    return res;
  }
  std::vector<std::pair<std::string, std::string>> Node::getAllSubscribeTopics() {
    std::vector<std::pair<std::string, std::string>> res;
    for (auto& [topic,subscribe]:m_subscribes) {
      res.push_back({topic,subscribe->getType()});
    }
    return res;
  }
}
