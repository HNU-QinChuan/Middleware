#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include<jsoncpp/json/json.h>
#include <unordered_set>

namespace Hnu::Middleware {
  class TopicController{
  public:
    void handleGet(Request& req, Response& res) {
      std::unordered_set<std::string> topics;
      for(auto& [key, value]: MiddlewareManager::middlewareManager.m_publishes) {
        topics.insert(key);
      }
      for(auto& [key, value]: MiddlewareManager::middlewareManager.m_subscribes) {
        topics.insert(key);
      }
      Json::Value root;
      Json::Value jsonArray(Json::arrayValue);
      for (const auto& topic : topics) {
        jsonArray.append(topic);
      }
      root["topics"] = jsonArray;
      res.result(http::status::ok);
      Json::StreamWriterBuilder writer;
      res.body() = writeString(writer, root);
    }
  };
  CONTROLLER_REGISTER(TopicController,"/topic",http::verb::get,&TopicController::handleGet)

}