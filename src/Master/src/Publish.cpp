//
// Created by yc on 25-2-14.
//

#include "Publish.hpp"

#include <spdlog/spdlog.h>

#include "Node.hpp"
#include "MiddlewareManager.hpp"

namespace Hnu::Middleware {
  Publish::Publish(const std::string& name,int eventfd,std::shared_ptr<Node> node):m_ioc(MiddlewareManager::getIoc()),m_name(name),m_eventfd(eventfd) ,m_node(node){
  }
  std::string Publish::getName() {
    return m_name;
  }
  void Publish::setNode(std::shared_ptr<Node> node) {
    m_node=node;
  }
  bool Publish::run() {
    auto pidfd = syscall(SYS_pidfd_open, m_node.lock()->getPid(), 0);
    if(pidfd==-1){
      return false;
    }
    m_eventfd= syscall(SYS_pidfd_getfd, pidfd, m_eventfd, 0);
    if(m_eventfd==-1){
      return false;
    }
    std::string shmName=m_node.lock()->getName()+"."+m_name;
    try{
      interprocess::shared_memory_object::remove(shmName.c_str());
      m_shm=interprocess::managed_shared_memory(interprocess::create_only,shmName.c_str(),SHM_SIZE);
      queue=m_shm.construct<lock_free_queue>(shmName.c_str())();
    }catch (const interprocess::interprocess_exception& e){
      spdlog::error("publish create shm error: {}",e.what());
      return false;
    }
    m_eventfdStream=std::make_unique<asio::posix::stream_descriptor>(m_ioc,m_eventfd);
    doEventfdRead();
    return true;
  }
  void Publish::doEventfdRead() {
    m_eventfdStream->async_read_some(asio::buffer(&m_eventfdValue,sizeof(m_eventfdValue)),std::bind_front(&Publish::onEventfdRead,shared_from_this()));
  }
  void Publish::onEventfdRead(const boost::system::error_code& ec,std::size_t bytes) {
    doEventfdRead();
    if(ec){
      spdlog::error("Eventfd Read Error: {}",ec.message());
      return;
    }
    string message(m_shm.get_segment_manager());
    queue->pop(message);
    spdlog::info("Publish: {}",message);
  }

}
