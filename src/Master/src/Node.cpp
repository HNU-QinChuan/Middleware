//
// Created by yc on 25-2-14.
//

#include "Node.hpp"

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
    publish->setNode(shared_from_this());
  }
}
