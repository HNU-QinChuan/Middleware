#include "interface/InterfaceRouter.hpp"
#include "InterfaceManager.hpp"

namespace Hnu::Interface {
  class WeightController {
  public:
    void handlePost(Request& req){
      std::string host1=req["host1"].to_string();
      std::string host2=req["host2"].to_string();
      std::string type=req["type"].to_string();
      if(type=="max"){
        InterfaceManager::interfaceManager.setMaxWeight(host1,host2);
      }else if(type=="init"){
        InterfaceManager::interfaceManager.setInitWeight(host1,host2);
      }
    }
  };

    CONTROLLER_REGISTER(WeightController, "/weight"
    ,http::verb::post,&WeightController::handlePost)
}