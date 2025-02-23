//
// Created by yc on 25-2-14.
//


#ifndef NODE_HPP
#define NODE_HPP

#include <memory>
#include <unordered_map>


namespace Hnu::Middleware {
  class Publish;
  class Subscribe;
  //TODO: 不使用共享指针
  class Node:std::enable_shared_from_this<Node> {
  public:
    Node(const std::string& name,pid_t pid);
    std::string getName();
    pid_t getPid();
    bool containsPublish(const std::string& topic);
    bool containsSubscribe(const std::string& topic);
    void addPublish(std::shared_ptr<Publish> publish) ;
    void addSubscribe(std::shared_ptr<Subscribe> subscribe);
    //TODO: 实现删除发布和订阅
    // void removePublish(std::string topic);
    // void removeSubscribe(std::string topic);
  private:
    std::string m_name;
    pid_t m_pid;
    //目前不能一个节点针对一个topic创建多个发布者或订阅者
    std::unordered_map<std::string,std::shared_ptr<Publish>> m_publishes;
    std::unordered_map<std::string,std::shared_ptr<Subscribe>> m_subscribes;
  };

}

#endif //NODE_HPP
