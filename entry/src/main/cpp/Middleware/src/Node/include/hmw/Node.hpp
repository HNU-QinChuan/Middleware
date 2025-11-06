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

#include "hilog/log.h"
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200
#define LOG_TAG "runCommand"
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
    void stop();
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
//         try {
//             OH_LOG_ERROR(LOG_APP, "[runCommand]createPublisher called, topic = %{public}s", topic.c_str());
//
//         if (m_impl.containsPublisher(topic)) {
//             OH_LOG_ERROR(LOG_APP, "[runCommand]Publisher for topic %{public}s already exists", topic.c_str());
//             return std::static_pointer_cast<Publisher<Message>>(m_impl.getPublisher(topic));
//         }
//
//         OH_LOG_ERROR(LOG_APP, "[runCommand]Creating Publisher for topic: %{public}s", topic.c_str());
//         auto publish = std::make_shared<Publisher<Message>>(m_ioc, &m_impl, topic);
//
//         OH_LOG_ERROR(LOG_APP, "[runCommand]Calling publish->run()");
//         if (!publish->run()) {
//             OH_LOG_ERROR(LOG_APP, "[runCommand]publish->run() failed for topic: %{public}s", topic.c_str());
//             spdlog::error("error on create publish: {}", topic);
//             return nullptr;
//         }
//
//         m_impl.addPublisher(topic, std::static_pointer_cast<PublisherInterface>(publish));
//         spdlog::debug("Create publish: {}", topic);
//         OH_LOG_ERROR(LOG_APP, "[runCommand]Publisher created successfully for topic: %{public}s", topic.c_str());
//             return publish;
//
//     } catch (const std::exception& e) {
//         OH_LOG_ERROR(LOG_APP, "[runCommand]Exception in createPublisher: %{public}s", e.what());
//         spdlog::error("Exception in createPublisher: {}", e.what());
//         return nullptr;
//     } catch (...) {
//         OH_LOG_ERROR(LOG_APP, "[runCommand]Unknown exception in createPublisher.");
//         spdlog::error("Unknown exception in createPublisher.");
//         return nullptr;
//     }
    }
    template <typename Message>
    std::shared_ptr<Publisher<Message>> createPublisher(const std::string& topic,const std::string& type){
//       if (m_impl.containsPublisher(topic)) {
//         return std::static_pointer_cast<Publisher<Message>>(m_impl.getPublisher(topic));
//       }
//       auto publish=std::make_shared<Publisher<Message>>(m_ioc,&m_impl,topic,type);
//       if(!publish->run()){
//         spdlog::error("error on create publish: {}",topic);
//         throw std::runtime_error("error on create publish");
//         // return nullptr;
//       }
//       m_impl.addPublisher(topic, std::static_pointer_cast<PublisherInterface>(publish));
//       spdlog::debug("Create publish: {}",topic);
//       return publish;
        try {
        OH_LOG_INFO(LOG_APP, "[createPublisher] Called, topic = %{public}s, type = %{public}s", topic.c_str(), type.c_str());

        if (m_impl.containsPublisher(topic)) {
            OH_LOG_WARN(LOG_APP, "[createPublisher] Publisher for topic %{public}s already exists", topic.c_str());
            return std::static_pointer_cast<Publisher<Message>>(m_impl.getPublisher(topic));
        }

        OH_LOG_INFO(LOG_APP, "[createPublisher] Creating Publisher for topic: %{public}s", topic.c_str());
        auto publish = std::make_shared<Publisher<Message>>(m_ioc, &m_impl, topic, type);

        OH_LOG_INFO(LOG_APP, "[createPublisher] Calling publish->run()");
        if (!publish->run()) {
            OH_LOG_ERROR(LOG_APP, "[createPublisher] publish->run() failed for topic: %{public}s", topic.c_str());
            throw std::runtime_error("error on create publish");
        }

        m_impl.addPublisher(topic, std::static_pointer_cast<PublisherInterface>(publish));
        OH_LOG_INFO(LOG_APP, "[createPublisher] Publisher created successfully for topic: %{public}s", topic.c_str());
        return publish;

    } catch (const std::exception& e) {
        OH_LOG_ERROR(LOG_APP, "[createPublisher] Exception: %{public}s", e.what());
        return nullptr;
    } catch (...) {
        OH_LOG_ERROR(LOG_APP, "[createPublisher] Unknown exception occurred.");
        return nullptr;
    }
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
    template <typename Message>
    std::shared_ptr<Subscriber<Message>> createSubscriber(const std::string& topic,const std::string& type,const std::function<void(std::shared_ptr<Message>)>& callback){
      if (m_impl.containsSubscriber(topic)) {
        return std::static_pointer_cast<Subscriber<Message>>(m_impl.getSubscriber(topic));
      }
      auto subscribe=std::make_shared<Subscriber<Message>>(m_ioc,&m_impl,topic,type);
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


