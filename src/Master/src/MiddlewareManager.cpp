//
// Created by yc on 25-2-14.
//

#include "MiddlewareManager.hpp"
#include "shm/Acceptor.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  MiddlewareManager MiddlewareManager::middlewareManager;
  // MiddlewareManager& MiddlewareManager::getInstance() {
  //   // static MiddlewareManager middlewareManager;
  //   return middlewareManager;
  // }
  asio::io_context& MiddlewareManager::getIoc() {
    return middlewareManager.m_ioc;
  }
  void MiddlewareManager::run() {
    Acceptor acceptor(middlewareManager.m_ioc);
    acceptor.run();
    middlewareManager.m_ioc.run();
  }
  void MiddlewareManager::transferMessage(const std::string& topic, const std::string& message) {
    auto iter=middlewareManager.m_subscribes.find(topic);
    if(iter==middlewareManager.m_subscribes.end()){
      return;
    }
    for (auto subscribe:iter->second) {
      subscribe->publish2Node(message);
    }
  }

}
