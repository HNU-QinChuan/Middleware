//
// Created by yc on 25-2-19.
//

#pragma once



#include<string>
#include<boost/asio.hpp>
#include<boost/lockfree/spsc_queue.hpp>
#include<boost/beast.hpp>
#include<boost/interprocess/containers/string.hpp>
#include<boost/interprocess/managed_shared_memory.hpp>
#include "PublisherInterface.hpp"


namespace Hnu::Middleware {
  class Node;
  namespace asio = boost::asio;
  namespace beast = boost::beast;
  namespace lockfree = boost::lockfree;
  namespace interprocess = boost::interprocess;

  constexpr std::size_t QUEUE_SIZE=10;

  using char_allocator = interprocess::allocator<char, interprocess::managed_shared_memory::segment_manager>;
  using string = interprocess::basic_string<char, std::char_traits<char>, char_allocator>;
  using lock_free_queue=lockfree::spsc_queue<string,lockfree::capacity<QUEUE_SIZE>>;
  using local_stream=beast::basic_stream<asio::local::stream_protocol>;

  template<typename Message>
  class Publisher :public PublisherInterface {
  public:
    Publisher(asio::io_context& ioc,std::shared_ptr<Node> node,const std::string& topic_name);
    bool run();
    void publish(const Message& message);
  private:


    asio::io_context& m_ioc;
    local_stream m_socket;
    std::string m_topic_name;
    std::weak_ptr<Node> m_node;
    int m_event_fd;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
  };

} // Middleware
// Hnu



