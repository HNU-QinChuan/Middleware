//
// Created by yc on 25-2-14.
//


#ifndef NODE_HPP
#define NODE_HPP

#include <memory>
#include <map>

#include"Subscribe.hpp"
#include"Publish.hpp"


namespace Hnu::Middleware {
  //TODO: 不使用共享指针
  class Node:std::enable_shared_from_this<Node> {
  public:
    Node(const std::string& name);
    // void addPublish(std::shared_ptr<Publish> publish) ;
    // void addSubscribe(std::shared_ptr<Subscribe> subscribe);
    //TODO: 实现删除发布和订阅
    // void removePublish(std::string topic);
    // void removeSubscribe(std::string topic);
  private:
    std::string m_name;
    std::map<std::string,std::shared_ptr<Publish>> m_publishes;
    std::map<std::string,std::shared_ptr<Subscribe>> m_subscribes;
  };

}

#endif //NODE_HPP
