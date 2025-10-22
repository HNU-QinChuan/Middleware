#include "GridFusion.hpp"
#include "Define.hpp"
#include <thread>


GridFusion::GridFusion(){
  map=new int8_t*[height];
  for(int i=0;i<height;++i)
    map[i]=new int8_t[width];
}

GridFusion::~GridFusion(){
  for(int i=0;i<height;++i)
    delete [] map[i];
  delete [] map;
}


void GridFusion::run(){
  grid_msg=std::make_shared<Nav::OccupancyGrid>();
  for(int i=0;i<height;++i){
    memset(map[i], 0, width*sizeof(int8_t));
  }
  // if(!bool_msg->data){
    point_handle.setMessage(point_msg, pose_msg);
    point_handle.run();
    map=point_handle.getMap();
  // }else{
  //   mmw_handle.setMessage(mmw_msg, pose_msg);
  //   mmw_handle.run();
  //   map=point_handle.getMap();
  // }
  // point_handle.setMessage(point_msg, pose_msg);
  // //std::thread t1{&PointHandle::run,&point_handle};
  // point_handle.run();
  // map=point_handle.getMap();
  // history_handle.setMessage(pose_msg);
  // std::thread t2{&HistoryHandle::run,&history_handle};

  // mmw_handle.setMessage(mmw_msg,pose_msg);
  // std::thread t3{&MmwHandle::run,&mmw_handle};
  //std::cout<<"run\n";
  // t1.join();
  // int8_t** map1=point_handle.getMap();

  // t2.join();
  // //int8_t** map2=history_handle.getMap();
  // int8_t** map2=map;
  
  // t3.join();
  // int8_t** map3=mmw_handle.getMap();


  // for(int j=0;j<height;++j){
  //   for(int i=0;i<width;++i){
  //     map[j][i]=map1[j][i]|map2[j][i]|map3[j][i];
  //   }
  // }
  //history_handle.LocalToGlobal(map);
  setGrid();

}

void GridFusion::setGrid(){
  grid_msg->mutable_header()->set_frame_id("map");
  // grid_msg->header.stamp=point_msg->header.stamp;
  // grid_msg->header.stamp.nanosec=0;
  // grid_msg->header.stamp.sec=0;
  //std::cout<<point_msg->header.stamp.sec<<' '<<point_msg->header.stamp.nanosec<<'\n';
  //std::cout<<"set grid\n";

  grid_msg->mutable_info()->set_height(height);
  grid_msg->mutable_info()->set_width(width);
  grid_msg->mutable_info()->set_resolution(resolution);
  grid_msg->mutable_info()->mutable_origin()->mutable_position()->CopyFrom(pose_msg->pose().position());
  grid_msg->mutable_info()->mutable_origin()->mutable_position()->set_x(pose_msg->pose().position().x()-actual_half_width);
  grid_msg->mutable_info()->mutable_origin()->mutable_position()->set_y(pose_msg->pose().position().y()-actual_half_height);
  grid_msg->mutable_info()->mutable_origin()->mutable_position()->set_z(0);
  //grid_msg->info.origin.orientation=pose_msg->pose.orientation;

  grid_msg->mutable_data()->reserve(height*width);
  for(int i=0;i<height;++i){
    for(int j=0;j<width;++j){
      grid_msg->mutable_data()->push_back(map[i][j]);
    }
  }
}
