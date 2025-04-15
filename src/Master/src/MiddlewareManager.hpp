//
// Created by yc on 25-2-14.
//


#pragma once

#include "shm/Publish.hpp"
#include "shm/Subscribe.hpp"
#include "shm/Node.hpp"
#include <jsoncpp/json/value.h>
#include<unordered_map>
#include<vector>
#include"ProcessManager.hpp"


namespace Hnu::Middleware {

  class MiddlewareManager {
  public:
    MiddlewareManager(const MiddlewareManager&)=delete;
    MiddlewareManager(MiddlewareManager&&)=delete;
    MiddlewareManager& operator=(const MiddlewareManager&)=delete;
    MiddlewareManager& operator=(MiddlewareManager&&)=delete;


    static void transferMessage(const std::string& topic,const std::string& message);
    static void transferInnerMessage(const std::string& topic,const std::string& message);
    static boost::asio::io_context& getIoc();
    static void run();
    static std::string getAllNodeInfo();

    std::unordered_map<std::string,std::shared_ptr<Node>> m_nodes;
    std::unordered_map<std::string,std::vector<std::shared_ptr<Publish>>> m_publishes;
    std::unordered_map<std::string,std::vector<std::shared_ptr<Subscribe>>> m_subscribes;
    // static MiddlewareManager& getInstance();
    static MiddlewareManager middlewareManager;
  private:
    MiddlewareManager();
    boost::asio::io_context m_ioc;
    ProcessManager processManager;

  };

}

