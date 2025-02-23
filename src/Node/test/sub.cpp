//
// Created by yc on 25-2-23.
//

#include <spdlog/spdlog.h>
#include "Node.hpp"
#include "Subscribe.hpp"
#include "proto/example.pb.h"

int main(){
  spdlog::set_level(spdlog::level::debug);
  auto node=std::make_shared<Hnu::Middleware::Node>("sub_node");
  auto subscribe=node->createSubscribe<Person>("topic1",[](std::shared_ptr<Person> person){
    spdlog::info("name: {}, id: {}, email: {}",person->name(),person->id(),person->email());
  });
  node->run();

}
