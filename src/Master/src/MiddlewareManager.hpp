//
// Created by yc on 25-2-14.
//


#ifndef MIDDLEWAREMANAGER_HPP
#define MIDDLEWAREMANAGER_HPP
#include"Publish.hpp"
#include"Subscribe.hpp"
#include<vector>
#include<map>


namespace Hnu::Middleware {

  class MiddlewareManager {
  public:
    MiddlewareManager(const MiddlewareManager&)=delete;
    MiddlewareManager(MiddlewareManager&&)=delete;
    MiddlewareManager& operator=(const MiddlewareManager&)=delete;
    MiddlewareManager& operator=(MiddlewareManager&&)=delete;
    static MiddlewareManager& getInstance();


    static bool addNode(const std::string& name);
  private:
    MiddlewareManager()=default;

    std::map<std::string,std::shared_ptr<Node>> m_nodes;
    std::map<std::string,std::shared_ptr<Publish>> m_publishes;
    std::map<std::string,std::shared_ptr<Subscribe>> m_subscribes;


  };

}

#endif //MIDDLEWAREMANAGER_HPP
