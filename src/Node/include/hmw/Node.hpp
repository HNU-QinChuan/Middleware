//
// Created by yc on 25-2-15.
//


#pragma once

#include <memory>
#include<boost/asio.hpp>
#include<boost/beast.hpp>
#include <spdlog/spdlog.h>
#include"hmw/Timer.hpp"
#include"hmw/NodeImpl.hpp"

namespace Hnu::Middleware {
  class PublisherInterface;
  template<typename Message>
  class Publisher;
  class SubscriberInterface;
  template<typename Message>
  class Subscriber;
  namespace asio=boost::asio;
  namespace beast=boost::beast;
  using local_stream=beast::basic_stream<asio::local::stream_protocol>;
  class Node : public std::enable_shared_from_this<Node> {
  public:
    Node(const std::string &name);
    void run();
    std::shared_ptr<Timer> createTimer(int interval, const std::function<void()>& callback);
    template <typename Message>
    std::shared_ptr<Publisher<Message>> createPublisher(const std::string& topic){
      if (m_impl.containsPublisher(topic)) {
        return std::static_pointer_cast<Publisher<Message>>(m_impl.getPublisher(topic));
      }
      auto publish=std::make_shared<Publisher<Message>>(m_ioc,&m_impl,topic);
      if(!publish->run()){
        spdlog::error("error on create publish: {}",topic);
        throw std::runtime_error("error on create publish");
        // return nullptr;
      }
      m_impl.addPublisher(topic, std::static_pointer_cast<PublisherInterface>(publish));
      spdlog::debug("Create publish: {}",topic);
      return publish;
    }
    template <typename Message>
    std::shared_ptr<Subscriber<Message>> createSubscriber(const std::string& topic,const std::function<void(std::shared_ptr<Message>)>& callback){
      if (m_impl.containsSubscriber(topic)) {
        return std::static_pointer_cast<Subscriber<Message>>(m_impl.getSubscriber(topic));
      }
      auto subscribe=std::make_shared<Subscriber<Message>>(m_ioc,&m_impl,topic);
      if(!subscribe->run(callback)){
        spdlog::error("error on create subscribe: {}",topic);
        throw std::runtime_error("error on create subscribe");
        // return nullptr;
      }
      m_impl.addSubscriber(topic, std::static_pointer_cast<SubscriberInterface>(subscribe));
      spdlog::debug("Create subscribe: {}",topic);
      return subscribe;
    }
    std::string getName();
  private:

    // std::string m_name;
    asio::io_context m_ioc;
    NodeImpl m_impl;
  };

} // Middleware
// Hnu


