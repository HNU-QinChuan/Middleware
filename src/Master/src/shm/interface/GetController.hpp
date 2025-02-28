//
// Created by yc on 25-2-28.
//

#pragma once

#include"shm/interface/ControllerInterface.hpp"
namespace Hnu::Middleware {
  class GetController : public Middleware::ControllerInterface {
  public:
    void handle(Request& req,Response& res) override{
      res.result(http::status::bad_request);
    }
  };

} // Middleware
