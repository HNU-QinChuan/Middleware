//
// Created by yc on 25-2-28.
//


#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"

namespace Hnu::Middleware {
  class NodeController  {
  public:
    void handlePost(Request& req, Response& res)  {
      pid_t pid = std::stoi(std::string(req["pid"]));
      std::string name = std::string(req["node"]);
      if (MiddlewareManager::addNode(name, pid)) {
        res.result(http::status::ok);
      } else {
        res.result(http::status::bad_request);
      }
    }
  };
  CONTROLLER_REGISTER(NodeController, "/node", http::verb::post,&NodeController::handlePost);
} // Middleware