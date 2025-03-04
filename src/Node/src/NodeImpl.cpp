//
// Created by yc on 25-3-4.
//

#include "hmw/NodeImpl.hpp"
#include <spdlog/spdlog.h>


namespace Hnu::Middleware {

  NodeImpl::NodeImpl(asio::io_context &ioc,const std::string& name) : m_ioc(ioc), m_socket(ioc) ,m_name(name),m_pid(getpid()) {

  }
  void NodeImpl::run() {
    boost::system::error_code ec;
    m_socket.connect("/tmp/master",ec);
    if(ec){
      spdlog::error("connect error: {}",ec.message());
      throw std::runtime_error("connect error");
    }
    beast::http::request<beast::http::empty_body> request;
    request.target("/node");
    request.method(beast::http::verb::post);
    request.set("node",m_name);
    request.set("pid",std::to_string(m_pid));
    request.prepare_payload();
    ec.clear();
    beast::http::write(m_socket,request,ec);
    if(ec){
      spdlog::error("write error: {}",ec.message());
      throw std::runtime_error("write error");
    }
    beast::http::response<beast::http::empty_body> response;
    beast::flat_buffer buffer;
    ec.clear();
    beast::http::read(m_socket,buffer,response,ec);
    if(ec){
      spdlog::error("read error: {}",ec.message());
      throw std::runtime_error("read error");
    }
    if (response.result()==beast::http::status::bad_request) {
      spdlog::error("error on server side");
      throw std::runtime_error("error on server side");
    }
    spdlog::debug("Create Node:{}",m_name);
  }
  bool NodeImpl::containsPublisher(const std::string& topic){
    return m_publishes.contains(topic);
  }
  bool NodeImpl::containsSubscriber(const std::string& topic){
    return m_subscribes.contains(topic);
  }
  void NodeImpl::addPublisher(const std::string& topic,std::shared_ptr<PublisherInterface> publisher){
    m_publishes[topic]=publisher;
  }
  void NodeImpl::addSubscriber(const std::string& topic,std::shared_ptr<SubscriberInterface> subscriber){
    m_subscribes[topic]=subscriber;
  }
  std::shared_ptr<SubscriberInterface> NodeImpl::getSubscriber(const std::string& topic){
    return m_subscribes[topic];
  }
  std::shared_ptr<PublisherInterface> NodeImpl::getPublisher(const std::string& topic){
    return m_publishes[topic];
  }
}