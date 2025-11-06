#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"
#include<jsoncpp/json/json.h>
#include <unordered_set>

namespace Hnu::Middleware {
  class TopicInfoController{
  public:
    void handleGet(Request& req, Response& res) {
      std::string topic_name = req["topic"].to_string();
      int pubcount = 0;
      int subcount = 0;
      if(MiddlewareManager::middlewareManager.m_publishes.contains(topic_name)) {
        pubcount = MiddlewareManager::middlewareManager.m_publishes[topic_name].size();
      }
      if(MiddlewareManager::middlewareManager.m_subscribes.contains(topic_name)) {
        subcount = MiddlewareManager::middlewareManager.m_subscribes[topic_name].size();
      }
      Json::Value root;
      Json::Value pubnodes(Json::arrayValue);
      Json::Value subnodes(Json::arrayValue);
      std::string topic_type;
      if(pubcount > 0) {
        for(auto& pub: MiddlewareManager::middlewareManager.m_publishes[topic_name]) {
          pubnodes.append(pub->getNodeName());
          if(topic_type.empty()) {
            topic_type = pub->getType();
          }
        }
      }
      if(subcount > 0) {
        for(auto& sub: MiddlewareManager::middlewareManager.m_subscribes[topic_name]) {
          subnodes.append(sub->getNodeName());
          if(topic_type.empty()) {
            topic_type = sub->getType();
          }
        }
      }
      for (auto &[hostName,hostPtr]:Interface::InterfaceManager::interfaceManager.hostlist) {
        auto subtopiclist=hostPtr->getNode2SubTopic2Type();
        for (auto& [nodeName,topic2type]:subtopiclist) {
          for (auto& [topic,type]:topic2type) {
            if (topic==topic_name) {
              subcount++;
              subnodes.append(nodeName);
              if(topic_type.empty()) {
                topic_type = type;
              }
            }
          }
        }
        auto pubtopiclist=hostPtr->getNode2PubTopic2Type();
        for (auto& [nodeName,topic2type]:pubtopiclist) {
          for (auto& [topic,type]:topic2type) {
            if (topic==topic_name) {
              pubcount++;
              pubnodes.append(nodeName);
              if(topic_type.empty()) {
                topic_type = type;
              }
            }
          }
        }
      }
      if(pubcount == 0 && subcount == 0) {
        res.result(http::status::bad_request);
        return;
      }
      root["topic"] = topic_name;
      root["type"] = topic_type;
      root["pubcount"] = pubcount;
      root["subcount"] = subcount;
      root["pubnodes"] = pubnodes;
      root["subnodes"] = subnodes;
      res.result(http::status::ok);
      Json::StreamWriterBuilder writer;
      res.body() = writeString(writer, root);
    }
  };
  CONTROLLER_REGISTER(TopicInfoController,"/topic/info",http::verb::get,&TopicInfoController::handleGet)

}