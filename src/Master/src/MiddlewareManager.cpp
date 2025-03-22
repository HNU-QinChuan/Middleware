//
// Created by yc on 25-2-14.
//

#include "MiddlewareManager.hpp"
#include "shm/Acceptor.hpp"
#include <jsoncpp/json/writer.h>
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  MiddlewareManager MiddlewareManager::middlewareManager;
  // MiddlewareManager& MiddlewareManager::getInstance() {
  //   // static MiddlewareManager middlewareManager;
  //   return middlewareManager;
  // }
  asio::io_context& MiddlewareManager::getIoc() {
    return middlewareManager.m_ioc;
  }
  void MiddlewareManager::run() {
    Acceptor acceptor(middlewareManager.m_ioc);
    acceptor.run();
    middlewareManager.m_ioc.run();
  }
  void MiddlewareManager::transferMessage(const std::string& topic, const std::string& message) {
    auto iter=middlewareManager.m_subscribes.find(topic);
    if(iter==middlewareManager.m_subscribes.end()){
      return;
    }
    for (auto subscribe:iter->second) {
      subscribe->publish2Node(message);
    }
  }
  void MiddlewareManager::getAllNodeInfo(asio::io_context& ioc,const std::function<void(std::string)>& callback){
    Json::Value root;
    Json::Value nodes(Json::arrayValue);
        /*
    {
      nodes:[
        {
          name:"node1",
          pubtopics:[
            {
              topic:"topic1",
              type:"type1"
            },
            {
              topic:"topic2",
              type:"type2"
            }
          ],
          subtopics:[
            {
              topic:"topic3",
              type:"type3"
            },
            {
              topic:"topic4",
              type:"type4"
            }
          ]
        }
      ]
    }
    */
    for(auto& [nodeName,nodeptr]:middlewareManager.m_nodes){
      Json::Value node;
      node["name"]=nodeName;
      Json::Value pubtopics(Json::arrayValue);
      std::vector<std::pair<std::string, std::string>> pubtopicsVec=nodeptr->getAllPublishTopics();
      for(auto& [topic,type]:pubtopicsVec){
        Json::Value topicJson;
        topicJson["topic"]=topic;
        topicJson["type"]=type;
        pubtopics.append(topicJson);
      }
      node["pubtopics"]=pubtopics;
      Json::Value subtopics(Json::arrayValue);
      std::vector<std::pair<std::string, std::string>> subtopicsVec=nodeptr->getAllSubscribeTopics();
      for(auto& [topic,type]:subtopicsVec){
        Json::Value topicJson;
        topicJson["topic"]=topic;
        topicJson["type"]=type;
        subtopics.append(topicJson);
      }
      node["subtopics"]=subtopics;
      nodes.append(node);
    }
    root["nodes"]=nodes;
    Json::StreamWriterBuilder writer;
    std::string jsonString = Json::writeString(writer, root);
    ioc.post(std::bind_front(callback, jsonString));
  }


}
