#pragma once

#include "base/BaseDwaPlanner.hpp"
#include "Nav/OccupancyGrid.pb.h"

namespace DWA{
  class CarDwaPlanner : public BaseDwaPlanner
  {
  public:
    CarDwaPlanner(const std::string &name);
    ~CarDwaPlanner();

  protected:
    // gridMap信息订阅者
    std::shared_ptr<Hnu::Middleware::Subscriber<Nav::OccupancyGrid>> grid_map_sub_;
    // 局部地图回掉函数
    //void map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);
    //0917 取消前看
    bool getLocalGoal();
    std::vector<std::vector<float>> raycast();
  };
}
