//
// Created by yc on 25-2-28.
//

#include "shm/interface/GetController.hpp"
#include "shm/UdsRouter.hpp"

namespace Hnu::Middleware {
  class NodeGetController : public GetController {
  public:
    void handle(Request& req, Response& res) override {
      res.result(http::status::ok);
    }
  };
  CONTROLLER_REGISTER(NodeGetController, "/node", http::verb::get)
} // Middleware