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
  class Publish:public std::enable_shared_from_this<Publish> {
  public:
    Publish(const std::string& name,int eventfd,std::shared_ptr<Node> node,const std::string& type);
    bool run();
    std::string getName();
    std::string getType();
    std::string getNodeName();
    void setNode(std::shared_ptr<Node> node);
    void cancel();
    ~Publish();
  private:
    void doEventfdRead();
    void onEventfdRead(const boost::system::error_code& ec,std::size_t bytes);

    std::string m_topic_name;
    std::weak_ptr<Node> m_node;
    std::string m_node_name;
    int m_eventfd;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    uint64_t m_eventfdValue;
    std::string m_type;

  };

}


