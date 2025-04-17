//
// Created by aoao on 25-4-16.
//

#include "car/CarDwaPlanner.hpp"
#include "base/BaseDwaPlanner.hpp"



int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  auto car_local_planner_node = std::make_shared<DWA::BaseDwaPlanner>("node");
  car_local_planner_node -> run();
  return 0;
}