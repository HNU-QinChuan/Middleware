//
// Created by yc on 25-2-14.
//


#pragma once

#include <memory>
#include<string>
#include"shm/Define.hpp"

namespace Hnu::Middleware {
  class Node;

  //TODO: 不使用共享指针
  class Subscribe : public std::enable_shared_from_this<Subscribe> {
  public:
    Subscribe(const std::string& name,int eventfd,std::shared_ptr<Node> node,const std::string& type);
    bool run();
    std::string getName();
    std::string getType();
    std::string getNodeName();
    void setNode(std::shared_ptr<Node> node);
    void publish2Node(const std::string& message);
    void cancle();
    ~Subscribe();
  private:

    std::weak_ptr<Node> m_node;
    std::string m_topic_name;
    int m_eventfd;
    std::string m_node_name;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    std::string m_type;
  };

} // Middleware
// Hnu


