#include "InterfaceManager.hpp"
#include"interface/InterfaceRouter.hpp"
#include "MiddlewareManager.hpp"




namespace Hnu::Interface {
  class PublishController{
  public:
    void handlePost(Request& req){
      std::string topic = std::string(req["topic"]);
      std::string data = req.body();
      Middleware::MiddlewareManager::transferInnerMessage(topic, data);
    }
  };
  CONTROLLER_REGISTER(PublishController, "/node/sub", http::verb::post, &PublishController::handlePost);
}