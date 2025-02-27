//
// Created by yc on 25-2-23.
//

#pragma once


#include"Subscriber.decl.hpp"
#include"Node.hpp"
#include<spdlog/spdlog.h>

namespace Hnu::Middleware {
  template<typename Message>
  Subscriber<Message>::Subscriber(asio::io_context& ioc, std::shared_ptr<Node> node, const std::string& topic_name)
    :m_ioc(ioc),m_socket(ioc),m_node(node),m_topic_name(topic_name),m_eventfdValue(0){
  }
  template<typename Message>
  bool Subscriber<Message>::run(const std::function<void(std::shared_ptr<Message>)>& callback) {
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
    request.target("/node/sub");
    request.method(beast::http::verb::post);
    request.set("sub",m_topic_name);
    request.set("node",m_node.lock()->getName());
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
    std::string shmName="sub."+m_node.lock()->getName()+"."+m_topic_name;

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
    m_callback=callback;
    m_eventfdStream->async_read_some(asio::buffer(&m_eventfdValue,sizeof(m_eventfdValue)),std::bind_front(&Subscriber::onRead,std::static_pointer_cast<Subscriber<Message>>(shared_from_this())));
    return true;
  }
  template<typename Message>
  void Subscriber<Message>::onRead(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    if(ec){
      spdlog::error("read error: {}",ec.message());
      return;
    }
    for (int i=0;i<m_eventfdValue;++i) {
      auto message=std::make_shared<Message>();
      string data{m_shm.get_segment_manager()};
      if(queue->pop(data)){
        message->ParseFromArray(data.data(),data.size());
        m_callback(message);
      }
    }
    // m_eventfdValue=0;
    m_eventfdStream->async_read_some(asio::buffer(&m_eventfdValue,sizeof(m_eventfdValue)),std::bind_front(&Subscriber::onRead,std::static_pointer_cast<Subscriber<Message>>(shared_from_this())));

  }
}


