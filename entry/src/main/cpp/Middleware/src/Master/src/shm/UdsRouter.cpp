//
// Created by yc on 25-2-28.
//

#include "UdsRouter.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  UdsRouter UdsRouter::m_instance;
  UdsRouter& UdsRouter::getInstance() {
    // static UdsRouter instance;
    return m_instance;
  }
  void UdsRouter::registerController(const std::string &path, http::verb verb, std::function<void (Request &, Response &)> func){
    getInstance().m_router[path][verb]=func;
  }
  void UdsRouter::handle(Request &req, Response &res) {
    //spdlog::debug("Handle request: target={}, method={}",std::string(req.target()), static_cast<int>(req.method()));

    auto& instance=getInstance();
    auto it=instance.m_router.find(req.target().to_string());
    if (it==instance.m_router.end()) {
      res.result(http::status::not_found);
      return;
    }
    auto it2=it->second.find(req.method());
    if (it2==it->second.end()) {
      res.result(http::status::not_found);
      return;
    }
//     if (!it2->second) {
//     spdlog::error("Handler function is empty for {} {}", std::string(req.target()), static_cast<int>(req.method()));
//     res.result(http::status::internal_server_error);
//     return;
// }
    it2->second(req,res);
  }
} // Middleware
// Hnu