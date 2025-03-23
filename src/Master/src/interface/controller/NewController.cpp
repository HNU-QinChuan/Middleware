#include "interface/InterfaceRouter.hpp"



namespace Hnu::Interface {
  class NewController{
  public:
    void handlepost(Request& req){

    }
  
  };
  CONTROLLER_REGISTER(NewController, "/node"
    ,http::verb::post,&NewController::handlepost)
}