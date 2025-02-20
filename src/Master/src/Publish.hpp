//
// Created by yc on 25-2-14.
//


#ifndef PUBLISH_HPP
#define PUBLISH_HPP

#include <memory>
#include<string>
#include<boost/interprocess/managed_shared_memory.hpp>
#include<boost/interprocess/containers/string.hpp>
#include<boost/lockfree/spsc_queue.hpp>
#include<boost/asio.hpp>
// #include"Node.hpp"

namespace Hnu::Middleware {
  constexpr std::size_t SHM_SIZE=1024*1024;
  constexpr std::size_t QUEUE_SIZE=10;
  namespace interprocess=boost::interprocess;
  namespace lockfree=boost::lockfree;
  namespace asio = boost::asio;
  using char_allocator = interprocess::allocator<char, interprocess::managed_shared_memory::segment_manager>;
  using string = interprocess::basic_string<char, std::char_traits<char>, char_allocator>;
  using lock_free_queue=lockfree::spsc_queue<string,lockfree::capacity<QUEUE_SIZE>>;
  class Node;
  //TODO: 不使用共享指针
  class Publish:public std::enable_shared_from_this<Publish> {
  public:
    Publish(const std::string& name,int eventfd,std::shared_ptr<Node> node);
    bool run();
    std::string getName();
    void setNode(std::shared_ptr<Node> node);
  private:
    void doEventfdRead();
    void onEventfdRead(const boost::system::error_code& ec,std::size_t bytes);

    asio::io_context& m_ioc;
    std::string m_topic_name;
    std::weak_ptr<Node> m_node;
    int m_eventfd;
    interprocess::managed_shared_memory m_shm;
    lock_free_queue* queue;
    std::unique_ptr<asio::posix::stream_descriptor> m_eventfdStream;
    uint64_t m_eventfdValue;

  };

}

#endif //PUBLISH_HPP
