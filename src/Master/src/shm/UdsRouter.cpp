//
// Created by yc on 25-2-28.
//

#include "UdsRouter.hpp"


namespace Hnu::Middleware {
  UdsRouter& UdsRouter::getInstance() {
    static UdsRouter instance;
    return instance;
  }
  void UdsRouter::registerController(const std::string &path, http::verb verb, std::function<void (Request &, Response &)> func){
    getInstance().m_router[path][verb]=func;
  }
  void UdsRouter::handle(Request &req, Response &res) {
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
    it2->second(req,res);
  }
} // Middleware
// Hnu