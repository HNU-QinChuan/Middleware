//
// Created by yc on 25-2-14.
//


#pragma once


#include <memory>
#include <unordered_map>
#include<vector>

namespace Hnu::Middleware {
  class Publish;
  class Subscribe;
  //TODO: 不使用共享指针
  class Node:std::enable_shared_from_this<Node> {
  public:
    Node(const std::string& name,pid_t pid);
    ~Node();
    std::string getName();
    pid_t getPid();
    bool containsPublish(const std::string& topic);
    bool containsSubscribe(const std::string& topic);
    void addPublish(std::shared_ptr<Publish> publish) ;
    void addSubscribe(std::shared_ptr<Subscribe> subscribe);

    std::vector<std::pair<std::string, std::string>> getAllPublishTopics();
    std::vector<std::pair<std::string, std::string>> getAllSubscribeTopics();

    std::shared_ptr<Publish> removePublish(const std::string& topic);
    std::shared_ptr<Subscribe> removeSubscribe(const std::string& topic);
    std::vector<std::shared_ptr<Publish>> removeAllPublish();
    std::vector<std::shared_ptr<Subscribe>> removeAllSubscribe();
  private:
    std::string m_name;
    pid_t m_pid;
    //目前不能一个节点针对一个topic创建多个发布者或订阅者
    std::unordered_map<std::string,std::shared_ptr<Publish>> m_publishes;
    std::unordered_map<std::string,std::shared_ptr<Subscribe>> m_subscribes;
  };

}


