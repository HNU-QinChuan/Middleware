//
// Created by yc on 25-2-23.
//

#ifndef DEFINE_HPP
#define DEFINE_HPP
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/asio.hpp>


namespace Hnu::Middleware {
  constexpr inline std::size_t SHM_SIZE=1024*1024;
  constexpr inline std::size_t QUEUE_SIZE=10;
  namespace interprocess=boost::interprocess;
  namespace lockfree=boost::lockfree;
  namespace asio = boost::asio;
  using char_allocator = interprocess::allocator<char, interprocess::managed_shared_memory::segment_manager>;
  using string = interprocess::basic_string<char, std::char_traits<char>, char_allocator>;
  using lock_free_queue=lockfree::spsc_queue<string,lockfree::capacity<QUEUE_SIZE>>;

}

#endif //DEFINE_HPP
