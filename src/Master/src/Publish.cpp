//
// Created by yc on 25-2-14.
//

#include "Publish.hpp"
#include "Node.hpp"
#include "MiddlewareManager.hpp"

namespace Hnu::Middleware {
  Publish::Publish(const std::string& name,int eventfd):m_ioc(MiddlewareManager::getIoc()),m_name(name),m_eventfd(eventfd) {
  }
  std::string Publish::getName() {
    return m_name;
  }
  void Publish::setNode(std::shared_ptr<Node> node) {
    m_node=node;
  }
  void Publish::run() {
    std::string shmName=m_node.lock()->getName()+"."+m_name;
    interprocess::shared_memory_object::remove(shmName.c_str());
    m_shm=interprocess::managed_shared_memory(interprocess::create_only,shmName.c_str(),SHM_SIZE);
    queue=m_shm.construct<lock_free_queue>(shmName.c_str())();
  }
}
