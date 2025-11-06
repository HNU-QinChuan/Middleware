//
// Created by yc on 25-3-2.
//

#pragma once
#include<boost/asio.hpp>
#include<boost/lockfree/spsc_queue.hpp>
#include<boost/beast.hpp>
#include<boost/interprocess/containers/string.hpp>
#include<boost/interprocess/managed_shared_memory.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace lockfree = boost::lockfree;
namespace interprocess = boost::interprocess;

constexpr inline std::size_t QUEUE_SIZE=10;

using char_allocator = interprocess::allocator<char, interprocess::managed_shared_memory::segment_manager>;
using string = interprocess::basic_string<char, std::char_traits<char>, char_allocator>;
using lock_free_queue=lockfree::spsc_queue<string,lockfree::capacity<QUEUE_SIZE>>;
using local_stream=beast::basic_stream<asio::local::stream_protocol>;