//
// Created by yc on 25-2-14.
//

#include "MiddlewareManager.hpp"

namespace Hnu::Middleware {
  MiddlewareManager& MiddlewareManager::getInstance() {
    static MiddlewareManager middlewareManager;
    return middlewareManager;
  }
  bool MiddlewareManager::addNode(const std::string& name) {
    if (getInstance().m_nodes.contains(name)) {
      return false;

    }
    getInstance().m_nodes[name]=std::make_shared<Node>(name);
  }
}
