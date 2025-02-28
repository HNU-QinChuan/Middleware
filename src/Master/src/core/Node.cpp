//
// Created by yc on 25-2-14.
//

#include "core/Node.hpp"
#include "core/Publish.hpp"
#include "core/Subscribe.hpp"

namespace Hnu::Middleware {
  Node::Node(const std::string& name,int pid):m_name(name),m_pid(pid) {
  }
  std::string Node::getName() {
    return m_name;
  }
  pid_t Node::getPid() {
    return m_pid;
  }
  bool Node::containsPublish(const std::string& topic) {
    return m_publishes.contains(topic);
  }
  bool Node::containsSubscribe(const std::string& topic) {
    return m_subscribes.contains(topic);
  }
  void Node::addPublish(std::shared_ptr<Publish> publish) {
    m_publishes[publish->getName()]=publish;
    // publish->setNode(shared_from_this());
  }
  void Node::addSubscribe(std::shared_ptr<Subscribe> subscribe) {
    m_subscribes[subscribe->getName()]=subscribe;
    // subscribe->setNode(shared_from_this());
  }
}
