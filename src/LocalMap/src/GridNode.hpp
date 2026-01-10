#pragma once
#include<hmw/Node.hpp>
#include<hmw/Publisher.hpp>
#include<hmw/Subscriber.hpp>
#include<hmw/Timer.hpp>
#include<Nav/OccupancyGrid.pb.h>
#include<Geometry/PoseStamped.pb.h>
#include<Sensor/PointCloud2.pb.h>
#include <boost/circular_buffer.hpp>
#include <mutex>
#include"GridFusion.hpp"



class GridNode:public Hnu::Middleware::Node{
public:
  GridNode();
  // void run();

  std::shared_ptr<GridNode> shared_from_this(){return std::static_pointer_cast<GridNode>(Hnu::Middleware::Node::shared_from_this());}

private:
  std::shared_ptr<Hnu::Middleware::Publisher<Nav::OccupancyGrid>> grid_pub;

  std::shared_ptr<Hnu::Middleware::Subscriber<Sensor::PointCloud2>> point_sub;
  std::shared_ptr<Hnu::Middleware::Subscriber<Geometry::PoseStamped>> pose_sub;
  std::shared_ptr<Hnu::Middleware::Subscriber<Nav::OccupancyGrid>> other_grid_sub;

  std::shared_ptr<Hnu::Middleware::Timer> timer;

  boost::circular_buffer<std::shared_ptr<Sensor::PointCloud2>> point_sub_msg;
  boost::circular_buffer<std::shared_ptr<Geometry::PoseStamped>> pose_sub_msg;
  boost::circular_buffer<std::shared_ptr<Nav::OccupancyGrid>> other_grid_sub_msg;

  std::mutex point_mutex;
  std::mutex pose_mutex;
  std::mutex mmw_mutex;
  std::mutex grid_mutex;
  std::mutex bool_mutex;
  std::mutex other_grid_mutex;

  std::shared_ptr<Nav::OccupancyGrid> grid_pub_msg;

  GridFusion algo;

private:
  void onPointSub(std::shared_ptr<Sensor::PointCloud2> point_message);
  void onPoseSub(std::shared_ptr<Geometry::PoseStamped> pose_message);
  void onOtherGridSub(std::shared_ptr<Nav::OccupancyGrid> other_grid_message);
  void onTime();
  void runFusion();


};