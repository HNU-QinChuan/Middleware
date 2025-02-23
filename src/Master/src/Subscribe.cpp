//
// Created by yc on 25-2-14.
//

#include "Subscribe.hpp"
#include "Node.hpp"
#include "MiddlewareManager.hpp"
#include<spdlog/spdlog.h>

namespace Hnu::Middleware {
  Subscribe::Subscribe(const std::string& name, int eventfd, std::shared_ptr<Node> node) : m_ioc(MiddlewareManager::getIoc()), m_topic_name(name), m_eventfd(eventfd), m_node(node) {

  }
  std::string Subscribe::getName() {
    return m_topic_name;
  }
  void Subscribe::setNode(std::shared_ptr<Node> node) {
    m_node = node;
  }
  bool Subscribe::run() {
    auto pidfd = syscall(SYS_pidfd_open, m_node.lock()->getPid(), 0);
    if(pidfd==-1){
      spdlog::error("pidfd open error: {}",strerror(errno));
      return false;
    }
    m_eventfd= syscall(SYS_pidfd_getfd, pidfd, m_eventfd, 0);
    if(m_eventfd==-1){
      spdlog::error("pidfd getfd error: {}",strerror(errno));
      return false;
    }
    std::string shmName=m_node.lock()->getName()+"."+m_topic_name;
    spdlog::debug("shmName: {}",shmName);
    try{
      interprocess::shared_memory_object::remove(shmName.c_str());
      m_shm=interprocess::managed_shared_memory(interprocess::create_only,shmName.c_str(),SHM_SIZE);
      queue=m_shm.construct<lock_free_queue>(shmName.c_str())();
    }catch (const interprocess::interprocess_exception& e){
      spdlog::error("publish create shm error: {}",e.what());
      return false;
    }
    m_eventfdStream=std::make_unique<asio::posix::stream_descriptor>(m_ioc,m_eventfd);
    return true;
  }
  void Subscribe::publish2Node(const std::string& message) {
    if (queue->push(string(message.c_str(),m_shm.get_segment_manager()))) {
      uint64_t value=1;
      m_eventfdStream->write_some(asio::buffer(&value,sizeof(value)));
    }
  }


}
