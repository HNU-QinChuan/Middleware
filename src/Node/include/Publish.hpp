//
// Created by yc on 25-2-19.
//

#ifndef PUBLISH_HPP
#define PUBLISH_HPP

#include"Publish.decl.hpp"
#include<spdlog/spdlog.h>
// #include <google/protobuf/message.h>




namespace Hnu::Middleware {
  template <typename Message>
  Publish<Message>::Publish(asio::io_context& ioc, const std::string& node_name, const std::string& topic_name)
    :m_ioc(ioc),m_socket(ioc),m_node_name(node_name),m_topic_name(topic_name){
  }
  template <typename Message>
  bool Publish<Message>::run(){
    m_event_fd=eventfd(0,0);
    if(m_event_fd==-1){
      spdlog::error("eventfd create error");
      return false;
    }
    m_eventfdStream=std::make_unique<asio::posix::stream_descriptor>(m_ioc,m_event_fd);

    boost::system::error_code ec;
    m_socket.connect("/tmp/master",ec);
    if(ec){
      spdlog::error("connect error: {}",ec.message());
      return false;
    }
    beast::http::request<beast::http::empty_body> request;
    request.target("/node/pub");
    request.method(beast::http::verb::post);
    request.set("pub",m_topic_name);
    request.set("node",m_node_name);
    request.set("eventfd",std::to_string(m_event_fd));
    request.prepare_payload();
    ec.clear();
    beast::http::write(m_socket,request,ec);
    if(ec){
      spdlog::error("write error: {}",ec.message());
      return false;
    }
    beast::http::response<beast::http::empty_body> response;
    beast::flat_buffer buffer;
    ec.clear();
    beast::http::read(m_socket,buffer,response,ec);
    if(ec){
      spdlog::error("read error: {}",ec.message());
      return false;
    }
    if (response.result()==beast::http::status::bad_request) {
      spdlog::error("error on server side");
      return false;
    }
    std::string shmName=m_node_name+"."+m_topic_name;
    try {
      m_shm=interprocess::managed_shared_memory(interprocess::open_only,shmName.c_str());
    }catch (const interprocess::interprocess_exception& e){
      spdlog::error("open shm error: {}",e.what());
      return false;
    }
    auto res=m_shm.find<lock_free_queue>(shmName.c_str());
    if(!res.second){
      spdlog::error("shm find error");
      return false;
    }
    queue=res.first;
    return true;
  }
  template <typename Message>
  void Publish<Message>::publish(const Message& message){
    std::string serialized_message;
    // serialized_message.reserve(message.ByteSizeLong());
    message.SerializeToString(&serialized_message);
    spdlog::debug("serialized message {}",serialized_message);
    if (queue->write_available()) {
      queue->push(string(serialized_message.c_str(),m_shm.get_segment_manager()));
      uint64_t one=1;
      m_eventfdStream->write_some(asio::buffer(&one,sizeof(one)));
    }
  }



}

#endif //PUBLISH_HPP
