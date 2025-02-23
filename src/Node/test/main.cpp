//
// Created by yc on 25-2-14.
//

#include<iostream>
#include "Node.hpp"
#include "Publish.hpp"
#include "proto/example.pb.h"

int main() {
  spdlog::set_level(spdlog::level::debug);
  auto node=std::make_shared<Hnu::Middleware::Node>("node1");

  Person person;
  auto publisher = node->createPublish<Person>("topic1");
  person.set_name("yc");
  person.set_id(1);
  person.set_email("1422776724");
  while (true) {
    publisher->publish(person);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  node->run();
  return 0;
}