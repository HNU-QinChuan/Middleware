//
// Created by yc on 25-2-28.
//


#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include"InterfaceManager.hpp"
#include<jsoncpp/json/json.h>

namespace Hnu::Middleware {
  class NodeController  {
  public:
    void handlePost(Request& req, Response& res)  {
      pid_t pid = std::stoi(std::string(req["pid"]));
      std::string name = std::string(req["node"]);
      if (addNode(name, pid)) {
        res.result(http::status::ok);
      } else {
        res.result(http::status::bad_request);
      }
    }
    bool addNode(const std::string& node,int pid) {
      if (MiddlewareManager::middlewareManager.m_nodes.find(node) != MiddlewareManager::middlewareManager.m_nodes.end()) {
        return false;
      }
      MiddlewareManager::middlewareManager.m_nodes[node]=std::make_shared<Node>(node,pid);
      Interface::InterfaceManager::addNode(node);
      spdlog::debug("Add Node: {}",node);
      return true;
    }
    void handleGet(Request& req, Response& res) {
      Json::Value root;
      Json::Value curretHostArray(Json::arrayValue);
      for (auto &pair: MiddlewareManager::middlewareManager.m_nodes) {
        curretHostArray.append(pair.first);
      }
      root[Interface::InterfaceManager::interfaceManager.m_hostName]=curretHostArray;
      for (auto &[hostName,hostPtr]:Interface::InterfaceManager::interfaceManager.hostlist) {
        Json::Value hostArray(Json::arrayValue);
        auto nodeList=hostPtr->getNodeList();
        for (auto &node:nodeList) {
          hostArray.append(node);
        }
        root[hostName]=hostArray;
      }
      res.result(http::status::ok);
      Json::StreamWriterBuilder writer;
      res.body() = writeString(writer, root);
    }
    void handleDelete(Request& req, Response& res) {
      std::string node_name = req["node"].to_string();
      std::shared_ptr<Node> node = MiddlewareManager::middlewareManager.m_nodes[node_name];
      if (node) {
        std::vector<std::shared_ptr<Subscribe>> subs=node->removeAllSubscribe();
        for (auto sub:subs) {
          sub->cancle();
          auto& vec=MiddlewareManager::middlewareManager.m_subscribes[sub->getName()];
          vec.erase(std::remove(vec.begin(),vec.end(),sub),vec.end());
          if(vec.empty()){
            MiddlewareManager::middlewareManager.m_subscribes.erase(sub->getName());
          }
        }
        std::vector<std::shared_ptr<Publish>> pubs=node->removeAllPublish();
        for (auto pub:pubs) {
          pub->cancel();
          auto& vec=MiddlewareManager::middlewareManager.m_publishes[pub->getName()];
          vec.erase(std::remove(vec.begin(),vec.end(),pub),vec.end());
          if(vec.empty()){
            MiddlewareManager::middlewareManager.m_publishes.erase(pub->getName());
          }
        }
        MiddlewareManager::middlewareManager.m_nodes.erase(node_name);
        res.result(http::status::ok);
        Interface::InterfaceManager::deleteNode(node_name);
      } else {
        res.result(http::status::bad_request);
      }
      
    }
  };
  CONTROLLER_REGISTER(NodeController, "/node"
    ,http::verb::post,&NodeController::handlePost
    ,http::verb::get,&NodeController::handleGet
    ,http::verb::delete_,&NodeController::handleDelete)
} // Middleware