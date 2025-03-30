#include "interface/InterfaceRouter.hpp"
#include "InterfaceManager.hpp"
#include<jsoncpp/json/json.h>
#include "MiddlewareManager.hpp"


namespace Hnu::Interface {
  class NewController{
  public:
    void handlePost(Request& req){
      std::string host=req["src"].to_string();
      std::string data=req.body();
      for(auto& [topic,hostset]:InterfaceManager::interfaceManager.topic2host){
        hostset.erase(host);
      }
      InterfaceManager::interfaceManager.hostlist[host]->clear();
      Json::Value root;
      Json::Reader reader;
      reader.parse(data, root);
      for (const auto& node : root["nodes"]) {
        std::string nodeName = node["name"].asString();
        InterfaceManager::interfaceManager.hostlist[host]->addNode(nodeName);
        for (const auto& sub : node["subtopics"]) {
          std::string topic = sub["topic"].asString();
          std::string type = sub["type"].asString();
          InterfaceManager::interfaceManager.hostlist[host]->addSub(nodeName, topic, type);
          InterfaceManager::interfaceManager.topic2host[topic].insert(host);
        }
        for (const auto& pub : node["pubtopics"]) {
          std::string topic = pub["topic"].asString();
          std::string type = pub["type"].asString();
          InterfaceManager::interfaceManager.hostlist[host]->addPub(nodeName, topic, type);
        }
      }
      http::request<http::string_body> re{
        http::verb::put,
        "/new",
        11
      };
      re.body()=Middleware::MiddlewareManager::getAllNodeInfo();
      re.set("src",InterfaceManager::interfaceManager.m_hostName);
      re.set("dest",host);
      InterfaceManager::interfaceManager.transfer(host,re);

    }
    void handlePut(Request& req){
      std::string host=req["src"].to_string();
      std::string data=req.body();
      for(auto& [topic,hostset]:InterfaceManager::interfaceManager.topic2host){
        hostset.erase(host);
      }
      InterfaceManager::interfaceManager.hostlist[host]->clear();
      Json::Value root;
      Json::Reader reader;
      reader.parse(data, root);
      for (const auto& node : root["nodes"]) {
        std::string nodeName = node["name"].asString();
        InterfaceManager::interfaceManager.hostlist[host]->addNode(nodeName);
        for (const auto& sub : node["subtopics"]) {
          std::string topic = sub["topic"].asString();
          std::string type = sub["type"].asString();
          InterfaceManager::interfaceManager.hostlist[host]->addSub(nodeName, topic, type);
          InterfaceManager::interfaceManager.topic2host[topic].insert(host);
        }
        for (const auto& pub : node["pubtopics"]) {
          std::string topic = pub["topic"].asString();
          std::string type = pub["type"].asString();
          InterfaceManager::interfaceManager.hostlist[host]->addPub(nodeName, topic, type);
        }
      }
    }
  
  };
  CONTROLLER_REGISTER(NewController, "/new"
    ,http::verb::post,&NewController::handlePost
    ,http::verb::put,&NewController::handlePut)
}