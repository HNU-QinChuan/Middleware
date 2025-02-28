//
// Created by yc on 25-2-28.
//


#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"

namespace Hnu::Middleware {
  class NodeSubController  {
  public:
    void handlePost(Request& req, Response& res)  {
      int eventfd = std::stoi(std::string(req["eventfd"]));
      std::string topic = std::string(req["sub"]);
      std::string node = std::string(req["node"]);
      if (MiddlewareManager::addSubscrie(node, topic, eventfd)) {
        res.result(http::status::ok);
      } else {
        res.result(http::status::bad_request);
      }
    }
  };
  CONTROLLER_REGISTER(NodeSubController, "/node/sub", http::verb::post,&NodeSubController::handlePost);
} // Middleware