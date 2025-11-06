//
// Created by yc on 25-2-15.
//

#include "hmw/Node.hpp"
#include <spdlog/spdlog.h>
#include "hilog/log.h"
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200
#define LOG_TAG "Node"

namespace Hnu::Middleware {

  Node::Node(const std::string &name) : m_impl(name){
    m_impl.run();
  }
  void Node::run(){
    asio::io_context::work work(m_ioc);
    m_ioc.run();
    //OH_LOG_DEBUG(LOG_APP,"[stopCommand] run exiting...");
  }
    
  void Node::stop(){
    m_ioc.stop();
    //OH_LOG_DEBUG(LOG_APP,"[stopCommand] stop...");
  }

  std::string Node::getName(){
    return m_impl.getName();
  }
  std::shared_ptr<Timer> Node::createTimer(int interval, const std::function<void()>& callback) {
    auto timer= std::make_shared<Timer>(m_ioc);
    timer->run(callback, interval);
    return timer;
  }


} // Middleware
// Hnu