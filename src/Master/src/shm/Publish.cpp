//
// Created by yc on 25-2-14.
//

#include "shm/Publish.hpp"

#include <spdlog/spdlog.h>
#include <sys/syscall.h>
#include "shm/Node.hpp"
#include "MiddlewareManager.hpp"


#ifndef SYS_pidfd_getfd
#define SYS_pidfd_getfd 438
#endif

namespace Hnu::Middleware {
  Publish::Publish(const std::string& name,int eventfd,std::shared_ptr<Node> node,const std::string& type):m_ioc(MiddlewareManager::getIoc()),
  m_topic_name(name),m_eventfd(eventfd) ,m_node(node),m_type(type){
  }
  std::string Publish::getName() {
    return m_topic_name;
  }
  void Publish::setNode(std::shared_ptr<Node> node) {
    m_node=node;
  }
  bool Publish::run() {
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
    std::string shmName="pub."+m_node.lock()->getName()+"."+m_topic_name;
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

    if(ec){
      spdlog::error("Eventfd Read Error: {}",ec.message());
      return;
    }
    for (int i=0;i<m_eventfdValue;++i) {
      string message(m_shm.get_segment_manager());
      queue->pop(message);
      std::string messageStr(message.data(),message.size());
      MiddlewareManager::transferMessage(m_topic_name,messageStr);
    }
    doEventfdRead();
  }

}
