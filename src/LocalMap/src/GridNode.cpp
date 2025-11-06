#include "GridNode.hpp"
#include <thread>
#include <google/protobuf/util/time_util.h>

GridNode::GridNode():Hnu::Middleware::Node{"Grid_node"},point_sub_msg{100},pose_sub_msg{100}{
  pose_sub_msg.push_back(std::make_shared<Geometry::PoseStamped>());
  point_sub_msg.push_back(std::make_shared<Sensor::PointCloud2>());
  point_sub=createSubscriber<Sensor::PointCloud2>("/ls128/lslidar_point_cloud", std::bind(&GridNode::onPointSub,shared_from_this(),std::placeholders::_1));
  pose_sub=createSubscriber<Geometry::PoseStamped>("/pose", std::bind(&GridNode::onPoseSub,shared_from_this(),std::placeholders::_1));

  grid_pub=createPublisher<Nav::OccupancyGrid>("/gridMap");
  timer=createTimer(50, std::bind(&GridNode::onTime,shared_from_this()));
  std::thread t{&GridNode::runFusion,shared_from_this()};
  t.detach();

}



void GridNode::onTime(){
  if(!grid_pub_msg)return;
  grid_mutex.lock();
  auto grid_msg=*grid_pub_msg;
  grid_mutex.unlock();

  *grid_msg.mutable_header()->mutable_stamp()=google::protobuf::util::TimeUtil::GetCurrentTime();
  grid_pub->publish(grid_msg);
  //std::cout<<"publish\n";
}

void GridNode::onPointSub(std::shared_ptr<Sensor::PointCloud2> point_message){
  //std::cout<<"sub point\n";
  std::lock_guard<std::mutex> lock(point_mutex);
  point_sub_msg.push_back(point_message);
}

void GridNode::onPoseSub(std::shared_ptr<Geometry::PoseStamped> pose_message){
  //std::cout<<"sub pose\n";
  std::lock_guard<std::mutex> lock(pose_mutex);
  pose_sub_msg.push_back(pose_message);
}


void GridNode::runFusion(){
  std::this_thread::sleep_for(std::chrono::seconds{1});
  while (true) {
    //if(pose_sub_msg.empty()||point_sub_msg.empty()||mmw_sub_msg.empty())continue;
	 // std::cout<<"once\n";
    pose_mutex.lock();
    auto pose_message=pose_sub_msg.back();
    pose_mutex.unlock();

    point_mutex.lock();
    auto point_message=point_sub_msg.back();
    point_mutex.unlock();

    algo.setMessage(point_message, pose_message);
    algo.run();

    grid_mutex.lock();
    grid_pub_msg=algo.getGrid();
    grid_mutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  }

}


