//
// Created by yc on 25-3-4.
//

#include "hmw/NodeImpl.hpp"
#include <spdlog/spdlog.h>


namespace Hnu::Middleware {

  NodeImpl::NodeImpl(const std::string& name) : m_socket(m_ioc)
   ,m_name(name),m_pid(getpid()),m_signalSet(m_ioc) {

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
    m_signalSet.add(SIGINT);
    m_signalSet.async_wait(std::bind_front(&NodeImpl::onSignal,this));
    std::thread t([this](){
      m_ioc.run();
    });
    t.detach();
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
  void NodeImpl::onSignal(const boost::system::error_code& e,int signal){
    if(e){
      spdlog::error("signal error: {}",e.message());
      throw std::runtime_error("signal error");
    }
    // m_signalSet.cancel();
    boost::system::error_code ec;
    m_socket.close();
    m_socket.connect("/tmp/master",ec);
    if(ec){
      spdlog::error("connect error: {}",ec.message());
      throw std::runtime_error("connect error");
    }
    beast::http::request<beast::http::empty_body> request;
    request.target("/node");
    request.method(beast::http::verb::delete_);
    request.set("node",m_name);
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
    m_socket.close();
    std::exit(0);
  }
}