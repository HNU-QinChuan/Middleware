//
// Created by yc on 25-2-19.
//

#pragma once


#include"hmw/Publisher.decl.hpp"
#include"hmw/Node.hpp"
#include<spdlog/spdlog.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp;

#include "hilog/log.h"
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200
#define LOG_TAG "hmwPublisher"

namespace Hnu::Middleware {
  template <typename Message>
  Publisher<Message>::Publisher(asio::io_context& ioc,NodeImpl* node, const std::string& topic_name)
    :m_ioc(ioc),m_socket(ioc),m_node(node),m_topic_name(topic_name){
    auto des=Message::descriptor();
    m_type=des->full_name();
  }
  template <typename Message>
  Publisher<Message>::Publisher(asio::io_context& ioc,NodeImpl* node, const std::string& topic_name,const std::string& type)
    :m_ioc(ioc),m_socket(ioc),m_node(node),m_topic_name(topic_name),m_type(type){
  }
  template <typename Message>
  bool Publisher<Message>::run(){
    m_event_fd=eventfd(0,0);
    //m_event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    int err = errno;
    if(m_event_fd==-1){
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]eventfd failed, errno: %{public}d", err);
      spdlog::error("eventfd create error");
      return false;
    }
    m_eventfdStream=std::make_unique<asio::posix::stream_descriptor>(m_ioc,m_event_fd);

    boost::system::error_code ec;
    //m_socket.connect("/data/storage/el2/base/files/master.sock",ec);
    tcp::resolver resolver(m_ioc);
    auto endpoints = resolver.resolve("127.0.0.1", "8080", ec);
    if (ec) {
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]resolver error: %{public}s", ec.message().c_str());
      spdlog::error("resolver error: {}", ec.message());
      return false;
    }

    asio::connect(m_socket, endpoints, ec);
    if(ec){
      OH_LOG_ERROR(LOG_APP, "[Publisher]connect error: %{public}s", ec.message().c_str());
      spdlog::error("connect error: {}",ec.message());
      return false;
    }
    beast::http::request<beast::http::empty_body> request;
    request.target("/node/pub");
    request.method(beast::http::verb::post);
    request.set("pub",m_topic_name);
    request.set("node",m_node->getName());
    request.set("eventfd",std::to_string(m_event_fd));
    request.set("type",m_type);
    request.prepare_payload();
    ec.clear();
    beast::http::write(m_socket,request,ec);
    if(ec){
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]write error: %{public}s", ec.message().c_str());
      spdlog::error("write error: {}",ec.message());
      return false;
    }
    beast::http::response<beast::http::empty_body> response;
    beast::flat_buffer buffer;
    ec.clear();
    beast::http::read(m_socket,buffer,response,ec);
    if(ec){
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]read error: %{public}s", ec.message().c_str());
      spdlog::error("read error: {}", ec.message());
      return false;
    }
    if (response.result()==beast::http::status::bad_request) {
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]error on server side");
      spdlog::error("error on server side");
      return false;
    }
    std::string shmName="pub."+m_node->getName()+"."+m_topic_name;
    OH_LOG_ERROR(LOG_APP, "[hmwPublisher]shm: %{public}s", shmName.c_str());
    try {
      m_shm=interprocess::managed_shared_memory(interprocess::open_only,shmName.c_str());
    }catch (const interprocess::interprocess_exception& e){
      //spdlog::error("open shm error: {}",e.what());
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]open shm error: %{public}s", e.what());
      return false;
    }
    auto res=m_shm.find<lock_free_queue>(shmName.c_str());
    if(!res.second){
      spdlog::error("shm find error");
      OH_LOG_ERROR(LOG_APP, "[hmwPublisher]shm find error");
      return false;
    }
    queue=res.first;
    return true;
  }
  template <typename Message>
  void Publisher<Message>::publish(const Message& message){
    string serialized_message(m_shm.get_segment_manager());
    serialized_message.resize(message.ByteSizeLong());
    message.SerializeToArray(serialized_message.data(),serialized_message.size());
    if (queue->write_available()) {
      queue->push(serialized_message);
      uint64_t one=1;
      m_eventfdStream->write_some(asio::buffer(&one,sizeof(one)));
    }
  }
}


