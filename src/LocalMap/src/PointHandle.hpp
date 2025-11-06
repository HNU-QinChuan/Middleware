#pragma once
#include<Sensor/PointCloud2.pb.h>
#include<Geometry/PoseStamped.pb.h>
#include <pcl/conversions.h>
#include<pcl/point_cloud.h>
#include<pcl/point_types.h>
#include<pcl/PCLPointCloud2.h>
class PointHandle{
public:
  PointHandle();
  ~PointHandle();
  void run();
  inline void setMessage(std::shared_ptr<Sensor::PointCloud2> point_message,std::shared_ptr<Geometry::PoseStamped> pose_messsage){
    point_msg=point_message;
    pose_msg=pose_messsage;
  }
  inline int8_t** getMap(){return map;}
private:
  std::shared_ptr<Sensor::PointCloud2> point_msg;
  std::shared_ptr<Geometry::PoseStamped> pose_msg;
  int8_t** map;
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
private:
  void Rotate();
  void setMap();
  void algorithm();
  template<typename T>
  void convertMsgToPCL(Sensor::PointCloud2 &cloud, pcl::PointCloud<T> &pcl_cloud){
    pcl::PCLPointCloud2 pcl_pc2;
    pcl_pc2.data=std::vector<uint8_t>{cloud.data().begin(),cloud.data().end()};
    pcl_pc2.width=cloud.width();
    pcl_pc2.height=cloud.height();
    pcl_pc2.is_dense=cloud.is_dense();
    pcl_pc2.is_bigendian=cloud.is_bigendian();
    pcl_pc2.point_step=cloud.point_step();
    pcl_pc2.row_step=cloud.row_step();
    pcl_pc2.header.frame_id=cloud.header().frame_id();
    pcl_pc2.header.seq=0;
    pcl_pc2.header.stamp=static_cast<std::uint64_t>(cloud.header().stamp().seconds()*1000000ull+cloud.header().stamp().nanos()/1000ull);
    pcl_pc2.fields.resize(cloud.fields().size());
    for(int i=0;i<cloud.fields().size();++i){
      pcl_pc2.fields[i].name=cloud.fields(i).name();
      pcl_pc2.fields[i].offset=cloud.fields(i).offset();
      pcl_pc2.fields[i].datatype=cloud.fields(i).datatype();
      pcl_pc2.fields[i].count=cloud.fields(i).count();
    }
    pcl::fromPCLPointCloud2(pcl_pc2,pcl_cloud);
  }
};
