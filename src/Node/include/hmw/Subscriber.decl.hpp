//
// Created by yc on 25-2-23.
//

#pragma once

#include "hmw/SubscriberInterface.hpp"
#include"Define.hpp"

namespace Hnu::Middleware {
  class Node;


  template<typename Message>
  class Subscriber:public SubscriberInterface {
  public:
    Subscriber(asio::io_context& ioc,std::shared_ptr<Node> node,const std::string& topic_name);
    bool run(const std::function<void(std::shared_ptr<Message>)>& callback);
    // void createSubscribe(const std::function<void(std::shared_ptr<Message> message)>& callback);
  private:
    void onRead(const boost::system::error_code& ec,std::size_t bytes_transferred);

    asio::io_context& m_ioc;
    local_stream m_socket;
    std::string m_topic_name;
    std::weak_ptr<Node> m_node;
    int m_event_fd;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::function<void(std::shared_ptr<Message> message)> m_callback;
    uint64_t m_eventfdValue;
    std::string m_type;
  };

}


