#pragma once
#include<Nav/OccupancyGrid.pb.h>
#include<Geometry/PoseStamped.pb.h>
#include<Sensor/PointCloud2.pb.h>
#include <boost/circular_buffer.hpp>
#include "PointHandle.hpp"
#include<Nav/OccupancyGrid.pb.h>

class GridFusion{
public:
  GridFusion();
  ~GridFusion();
  inline void setMessage(
    std::shared_ptr<Sensor::PointCloud2> point_message,
    std::shared_ptr<Geometry::PoseStamped> pose_message,
    std::shared_ptr<Nav::OccupancyGrid> other_grid_message
  ){
    point_msg=point_message;
    pose_msg=pose_message;
    other_grid_msg=other_grid_message;
  }
  void run();
  std::shared_ptr<Nav::OccupancyGrid> getGrid(){return grid_msg;}
private:
  //boost::circular_buffer<Position> history_position;

  std::shared_ptr<Sensor::PointCloud2> point_msg;
  std::shared_ptr<Geometry::PoseStamped> pose_msg;
  std::shared_ptr<Nav::OccupancyGrid> other_grid_msg;

  std::shared_ptr<Nav::OccupancyGrid> grid_msg;

  int8_t** map;
  
  //HistoryHandle history_handle;
  PointHandle   point_handle;

private:
  void setGrid();
};