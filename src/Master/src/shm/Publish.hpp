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
    Publish(const std::string& name,int eventfd,std::shared_ptr<Node> node);
    bool run();
    std::string getName();
    void setNode(std::shared_ptr<Node> node);
  private:
    void doEventfdRead();
    void onEventfdRead(const boost::system::error_code& ec,std::size_t bytes);

    asio::io_context& m_ioc;
    std::string m_topic_name;
    std::weak_ptr<Node> m_node;
    int m_eventfd;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    uint64_t m_eventfdValue;

  };

}


