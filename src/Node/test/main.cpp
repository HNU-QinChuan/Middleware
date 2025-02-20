//
// Created by yc on 25-2-14.
//

#include<iostream>
#include "Node.hpp"
#include "proto/example.pb.h"

int main() {
  spdlog::set_level(spdlog::level::debug);
  Hnu::Middleware::Node node("node1");
  Person person;
  auto publisher = node.createPublish<Person>("topic1");
  person.set_name("yc");
  person.set_id(1);
  person.set_email("1422776724");
  publisher->publish(person);
  node.run();
  return 0;
}