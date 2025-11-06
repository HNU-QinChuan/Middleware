//
// Created by yc on 25-2-23.
//

#pragma once

#include "hmw/SubscriberInterface.hpp"
#include"Define.hpp"
#include "hmw/NodeImpl.hpp"
#include<google/protobuf/message.h>

namespace Hnu::Middleware {

  template<typename Message>
  class Subscriber:public SubscriberInterface {
  public:
    Subscriber(asio::io_context& ioc,NodeImpl* node,const std::string& topic_name);
    Subscriber(asio::io_context& ioc,NodeImpl* node,const std::string& topic_name,const std::string& type);
    bool run(const std::function<void(std::shared_ptr<Message>)>& callback);
    // void createSubscribe(const std::function<void(std::shared_ptr<Message> message)>& callback);
  private:
    void onRead(const boost::system::error_code& ec,std::size_t bytes_transferred);

    asio::io_context& m_ioc;
    local_stream m_socket;
    std::string m_topic_name;
    NodeImpl* m_node;
    int m_event_fd;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::function<void(std::shared_ptr<Message> message)> m_callback;
    uint64_t m_eventfdValue;
    std::string m_type;
    const google::protobuf::Message* m_prototype;
  };

}


