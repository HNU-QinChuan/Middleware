#include"interface/InterfaceRouter.hpp"
#include "MiddlewareManager.hpp"
#include<spdlog/spdlog.h>




namespace Hnu::Interface {
  class PublishController{
  public:
    void handlePost(Request& req){
      std::string topic = std::string(req["topic"]);
      std::string data = req.body();
      Middleware::MiddlewareManager::transferInnerMessage(topic, data);
      // spdlog::debug("get topic {} from host {}", topic, req["src"].to_string());
    }
  };
  CONTROLLER_REGISTER(PublishController, "/publish", http::verb::post, &PublishController::handlePost);
}