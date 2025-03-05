//
// Created by yc on 25-2-19.
//

#pragma once



#include "Define.hpp"
#include "hmw/PublisherInterface.hpp"


namespace Hnu::Middleware {
  class Node;

  template<typename Message>
  class Publisher :public PublisherInterface {
  public:
    Publisher(asio::io_context& ioc,std::shared_ptr<Node> node,const std::string& topic_name);
    bool run();
    void publish(const Message& message);
  private:
    asio::io_context& m_ioc;
    local_stream m_socket;
    std::string m_topic_name;
    std::weak_ptr<Node> m_node;
    int m_event_fd;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::string m_type;
  };

} // Middleware
// Hnu



