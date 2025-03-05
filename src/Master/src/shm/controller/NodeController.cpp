//
// Created by yc on 25-2-28.
//


#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
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
      if (MiddlewareManager::middlewareManager.m_nodes.contains(node)) {
        return false;
      }
      MiddlewareManager::middlewareManager.m_nodes[node]=std::make_shared<Node>(node,pid);
      spdlog::debug("Add Node: {}",node);
      return true;
    }
    void handleGet(Request& req, Response& res) {
      Json::Value root;
      Json::Value jsonArray(Json::arrayValue);
      for (auto &pair: MiddlewareManager::middlewareManager.m_nodes) {
        jsonArray.append(pair.first);
      }
      root["nodes"] = jsonArray;
      res.result(http::status::ok);
      Json::StreamWriterBuilder writer;
      res.body() = writeString(writer, root);
    }
    void handleDelete(Request& req, Response& res) {

    }
  };
  CONTROLLER_REGISTER(NodeController, "/node"
    , http::verb::post,&NodeController::handlePost
    ,http::verb::get,&NodeController::handleGet
    ,http::verb::delete_,&NodeController::handleDelete)
} // Middleware