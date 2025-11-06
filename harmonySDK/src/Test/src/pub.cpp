//
// Created by yc on 25-2-14.
//

#include<iostream>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <Std/String.pb.h>

class PubNode: public Hnu::Middleware::Node {
public:
  PubNode(const std::string &name):Node(name) {
    // Create a publisher
    publisher = createPublisher<Std::String>("topic1");
    // Create a timer
    timer = createTimer(1000, [this] { onTimer(); });
  }
  std::shared_ptr<Hnu::Middleware::Publisher<Std::String>> publisher;
  std::shared_ptr<Hnu::Middleware::Timer> timer;
  int i=0;
  void onTimer() {
    i++;
    Std::String str;
    str.set_data("from node1"+std::to_string(i));
    publisher->publish(str);
  }
};



int main() {

  spdlog::set_level(spdlog::level::debug);
  auto node = std::make_shared<PubNode>("pub_node");
  node->run();
  return 0;
}