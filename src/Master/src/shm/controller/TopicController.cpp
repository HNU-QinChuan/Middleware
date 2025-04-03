#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"
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
      root[Interface::InterfaceManager::interfaceManager.m_hostName]=jsonArray;

      for (auto &[hostName,hostPtr]:Interface::InterfaceManager::interfaceManager.hostlist) {
        Json::Value hostArray(Json::arrayValue);
        topics.clear();
        auto subtopiclist=hostPtr->getNode2SubTopic2Type();
        for (auto& [nodeName,topic2type]:subtopiclist) {
          for (auto& [topic,type]:topic2type) {
            topics.insert(topic);
          }
        }
        auto pubtopiclist=hostPtr->getNode2PubTopic2Type();
        for (auto& [nodeName,topic2type]:pubtopiclist) {
          for (auto& [topic,type]:topic2type) {
            topics.insert(topic);
          }
        }
        for (const auto& topic : topics) {
          hostArray.append(topic);
        }
        root[hostName]=hostArray;
      }
      res.result(http::status::ok);
      Json::StreamWriterBuilder writer;
      res.body() = writeString(writer, root);
    }
  };
  CONTROLLER_REGISTER(TopicController,"/topic",http::verb::get,&TopicController::handleGet)

}