//
// Created by yc on 25-2-14.
//


#ifndef MIDDLEWAREMANAGER_HPP
#define MIDDLEWAREMANAGER_HPP
#include"Publish.hpp"
#include"Subscribe.hpp"
#include"Node.hpp"
#include<unordered_map>
#include<vector>


namespace Hnu::Middleware {

  class MiddlewareManager {
  public:
    MiddlewareManager(const MiddlewareManager&)=delete;
    MiddlewareManager(MiddlewareManager&&)=delete;
    MiddlewareManager& operator=(const MiddlewareManager&)=delete;
    MiddlewareManager& operator=(MiddlewareManager&&)=delete;


    static bool addNode(const std::string& node,int pid);
    static bool addPublish(const std::string& node,const std::string& topic,int eventfd);
    static bool addSubscrie(const std::string& node, const std::string& topic, int eventfd);
    static void transferMessage(const std::string& topic,const std::string& message);
    static boost::asio::io_context& getIoc();
    static void run();
  private:
    MiddlewareManager()=default;
    static MiddlewareManager& getInstance();
    boost::asio::io_context m_ioc;
    std::unordered_map<std::string,std::shared_ptr<Node>> m_nodes;
    std::unordered_map<std::string,std::vector<std::shared_ptr<Publish>>> m_publishes;
    std::unordered_map<std::string,std::vector<std::shared_ptr<Subscribe>>> m_subscribes;
  };

}

#endif //MIDDLEWAREMANAGER_HPP
