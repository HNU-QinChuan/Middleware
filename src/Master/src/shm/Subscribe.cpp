//
// Created by yc on 25-2-14.
//

#include "shm/Subscribe.hpp"
#include "shm/Node.hpp"
#include "MiddlewareManager.hpp"
#include<spdlog/spdlog.h>
#include <sys/syscall.h>

#ifndef SYS_pidfd_getfd
#define SYS_pidfd_getfd 438
#endif

namespace Hnu::Middleware {
  Subscribe::Subscribe(const std::string& name, int eventfd, std::shared_ptr<Node> node,const std::string& type) :
  m_topic_name(name), m_eventfd(eventfd), m_node(node),m_type(type) {

  }
  Subscribe::~Subscribe() {
    spdlog::debug("Destroy Subscribe:{}",m_topic_name);
    // m_eventfdStream->close();
    // std::shared_ptr<Node> node=m_node.lock();
    // if(node){
    //   node->removePublish(m_topic_name);
    // }
    // interprocess::shared_memory_object::remove(("sub."+m_node_name+"."+m_topic_name).c_str());
    // MiddlewareManager::middlewareManager.m_subscribes.erase(m_topic_name);
  }
  void Subscribe::cancle() {
    m_eventfdStream->cancel();
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
    m_node_name=m_node.lock()->getName();
    std::string shmName="sub."+m_node_name+"."+m_topic_name;
    try{
      interprocess::shared_memory_object::remove(shmName.c_str());
      m_shm=interprocess::managed_shared_memory(interprocess::create_only,shmName.c_str(),SHM_SIZE);
      queue=m_shm.construct<lock_free_queue>(shmName.c_str())();
    }catch (const interprocess::interprocess_exception& e){
      spdlog::error("subscribe create shm error: {}",e.what());
      return false;
    }
    m_eventfdStream=std::make_unique<asio::posix::stream_descriptor>(MiddlewareManager::getIoc(),m_eventfd);
    return true;
  }
  void Subscribe::publish2Node(const std::string& message) {
    if (queue->push(string(message.data(),message.size(),m_shm.get_segment_manager()))) {
      uint64_t value=1;
      m_eventfdStream->write_some(asio::buffer(&value,sizeof(value)));
    }
  }
  std::string Subscribe::getType() {
    return m_type;
  }
  std::string Subscribe::getNodeName() {
    return m_node_name;
  }


}
