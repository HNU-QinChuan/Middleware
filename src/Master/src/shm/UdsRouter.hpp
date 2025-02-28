//
// Created by yc on 25-2-28.
//


#pragma once
#include<unordered_map>
#include<string>
#include<boost/beast/http.hpp>
#include <spdlog/spdlog.h>

#include "interface/ControllerInterface.hpp"

namespace Hnu::Middleware {
  namespace http=boost::beast::http;
  class UdsRouter {
  public:
    UdsRouter(const UdsRouter&)=delete;
    UdsRouter& operator=(const UdsRouter&)=delete;
    UdsRouter(const UdsRouter&&)=delete;
    UdsRouter& operator=(const UdsRouter&&)=delete;
    static void registerController(const std::string& path,http::verb verb,std::unique_ptr<ControllerInterface> controller) {
      spdlog::info("path {} ",path);
    }
  private:
    UdsRouter()=default;
    static UdsRouter udsRouter;
    std::unordered_map<std::string,std::unordered_map<http::verb,std::unique_ptr<ControllerInterface>>> m_router;
  };

  template<typename T>
  class Controllerregister {
  public:
    Controllerregister(const std::string& path,http::verb verb) {
      UdsRouter::registerController(path,verb,std::make_unique<T>());
    }
  };

  #define CONTROLLER_REGISTER(controller_class, path, method) \
  static Controllerregister<controller_class> registrar##controller_class(path,method)




} // Middleware
// Hnu


