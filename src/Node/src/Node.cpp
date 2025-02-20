//
// Created by yc on 25-2-15.
//

#include "Node.hpp"
#include <spdlog/spdlog.h>


namespace Hnu::Middleware {
  Node::Node(const std::string &name) : m_name(name) , m_socket(m_ioc), m_pid(getpid()){
    boost::system::error_code ec;
    m_socket.connect("/tmp/master",ec);
    if(ec){
      spdlog::error("connect error: {}",ec.message());
      throw std::runtime_error("connect error");
    }
    beast::http::request<beast::http::empty_body> request;
    request.target("/node");
    request.method(beast::http::verb::post);
    request.set("node",name);
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


  }
  void Node::run(){
    asio::io_context::work work(m_ioc);
    m_ioc.run();
  }
} // Middleware
// Hnu