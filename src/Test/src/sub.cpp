//
// Created by yc on 25-2-23.
//

#include <spdlog/spdlog.h>
#include <hmw/Node.hpp>
#include <hmw/Subscriber.hpp>
#include <Std/String.pb.h>

int main(){
  spdlog::set_level(spdlog::level::debug);
  auto node=std::make_shared<Hnu::Middleware::Node>("sub_node");
  auto subscribe=node->createSubscriber<Std::String>("test",[](std::shared_ptr<Std::String> person){
    spdlog::info("str: {}", person->data());
  });
  node->run();

}
