//
// Created by yc on 25-2-15.
//


#ifndef NODE_HPP
#define NODE_HPP
#include <memory>
#include<boost/asio.hpp>
#include<boost/beast.hpp>
#include <spdlog/spdlog.h>

namespace Hnu::Middleware {
  class PublishInterface;
  template<typename Message>
  class Publish;
  class SubscribeInterface;
  template<typename Message>
  class Subscribe;

  namespace asio=boost::asio;
  namespace beast=boost::beast;
  using local_stream=beast::basic_stream<asio::local::stream_protocol>;
  class Node : public std::enable_shared_from_this<Node> {
  public:
    Node(const std::string &name);
    void run();
    template <typename Message>
    std::shared_ptr<Publish<Message>> createPublish(const std::string& topic){
      if (m_publishes.contains(topic)) {
        return std::static_pointer_cast<Publish<Message>>(m_publishes[topic]);
      }
      auto publish=std::make_shared<Publish<Message>>(m_ioc,shared_from_this(),topic);
      if(!publish->run()){
        spdlog::error("error on create publish: {}",topic);
        throw std::runtime_error("error on create publish");
        // return nullptr;
      }
      m_publishes[topic]=publish;
      spdlog::debug("Create publish: {}",topic);
      return publish;
    }
    template <typename Message>
    std::shared_ptr<Subscribe<Message>> createSubscribe(const std::string& topic,const std::function<void(std::shared_ptr<Message>)>& callback){
      if (m_subscribes.contains(topic)) {
        return std::static_pointer_cast<Subscribe<Message>>(m_subscribes[topic]);
      }
      auto subscribe=std::make_shared<Subscribe<Message>>(m_ioc,shared_from_this(),topic);
      if(!subscribe->run(callback)){
        spdlog::error("error on create subscribe: {}",topic);
        throw std::runtime_error("error on create subscribe");
        // return nullptr;
      }
      m_subscribes[topic]=subscribe;
      spdlog::debug("Create subscribe: {}",topic);
      return subscribe;
    }
    std::string getName();
  private:
    std::string m_name;
    asio::io_context m_ioc;
    local_stream m_socket;
    pid_t m_pid;
    std::unordered_map<std::string,std::shared_ptr<PublishInterface>> m_publishes;
    std::unordered_map<std::string,std::shared_ptr<SubscribeInterface>> m_subscribes;

  };

} // Middleware
// Hnu

#endif //NODE_HPP
