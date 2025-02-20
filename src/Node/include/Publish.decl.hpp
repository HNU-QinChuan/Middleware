//
// Created by yc on 25-2-19.
//

#ifndef PUBLISH_DECL_HPP
#define PUBLISH_DECL_HPP


#include<string>
#include<boost/asio.hpp>
#include<boost/lockfree/spsc_queue.hpp>
#include<boost/beast.hpp>
#include<boost/interprocess/containers/string.hpp>
#include<boost/interprocess/managed_shared_memory.hpp>
#include<memory>

namespace Hnu::Middleware {
  namespace asio = boost::asio;
  namespace beast = boost::beast;
  namespace lockfree = boost::lockfree;
  namespace interprocess = boost::interprocess;

  constexpr std::size_t QUEUE_SIZE=10;

  using char_allocator = interprocess::allocator<char, interprocess::managed_shared_memory::segment_manager>;
  using string = interprocess::basic_string<char, std::char_traits<char>, char_allocator>;
  using lock_free_queue=lockfree::spsc_queue<string,lockfree::capacity<QUEUE_SIZE>>;
  using local_stream=beast::basic_stream<asio::local::stream_protocol, asio::any_io_executor, beast::unlimited_rate_policy>;

  template<typename Message>
  class Publish :public std::enable_shared_from_this<Publish<Message>> {
  public:
    Publish(asio::io_context& ioc,const std::string& node_name,const std::string& topic_name);
    bool run();
    void publish(const Message& message);
  private:


    asio::io_context& m_ioc;
    local_stream m_socket;
    std::string m_topic_name;
    std::string m_node_name;
    int m_event_fd;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
  };

} // Middleware
// Hnu


#endif //PUBLISH_DECL_HPP
