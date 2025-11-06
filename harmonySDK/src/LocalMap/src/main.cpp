#include<spdlog/spdlog.h>
#include "GridNode.hpp"

int main(){
  spdlog::info("LocalMap Node Started");
  auto node=std::make_shared<GridNode>();
  node->run();
}