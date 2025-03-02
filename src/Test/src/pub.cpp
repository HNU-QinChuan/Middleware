//
// Created by yc on 25-2-14.
//

#include<iostream>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <Std/String.pb.h>


int main() {

  spdlog::set_level(spdlog::level::debug);
  auto node=std::make_shared<Hnu::Middleware::Node>("node1");

  auto publisher = node->createPublisher<Std::String>("topic1");
  Std::String str;
  int i=0;
  while (true) {
    i++;
    str.set_data("hello world"+std::to_string(i));
    publisher->publish(str);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  node->run();
  return 0;
}