//
// Created by yc on 25-2-15.
//


#ifndef NODE_HPP
#define NODE_HPP
#include <memory>
#include<boost/asio.hpp>
#include<boost/beast.hpp>
#include "Publish.hpp"

namespace Hnu::Middleware {
  namespace asio=boost::asio;
  namespace beast=boost::beast;
  using local_stream=beast::basic_stream<asio::local::stream_protocol, asio::any_io_executor, beast::unlimited_rate_policy>;
  class Node : public std::enable_shared_from_this<Node> {
  public:
    Node(const std::string &name);
    void run();
    template <typename Message>
    std::shared_ptr<Publish<Message>> createPublish(const std::string& topic){
      auto publish=std::make_shared<Publish<Message>>(m_ioc,m_name,topic);
      if(!publish->run()){
        return nullptr;
      }
      spdlog::debug("create publish: {}",topic);
      return publish;
    }
  private:


    std::string m_name;
    asio::io_context m_ioc;
    local_stream m_socket;
    pid_t m_pid;

  };

} // Middleware
// Hnu

#endif //NODE_HPP
