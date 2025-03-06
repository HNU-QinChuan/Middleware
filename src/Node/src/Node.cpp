//
// Created by yc on 25-2-15.
//

#include "hmw/Node.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {



  Node::Node(const std::string &name) : m_name(name) ,m_impl(m_name){
    m_impl.run();
  }
  void Node::run(){
    asio::io_context::work work(m_ioc);
    m_ioc.run();
  }
  std::string Node::getName(){
    return m_name;
  }
  std::shared_ptr<Timer> Node::createTimer(int interval, const std::function<void()>& callback) {
    auto timer= std::make_shared<Timer>(m_ioc);
    timer->run(callback, interval);
    return timer;
  }


} // Middleware
// Hnu