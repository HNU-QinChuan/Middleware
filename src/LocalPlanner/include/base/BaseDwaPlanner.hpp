//
// Created by aoao on 25-4-8.
//

#pragma once

#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <TaskPlanner/GlobalPathPlan.pb.h>
#include <TaskPlanner/GlobalPathPlanFeedback.pb.h>
#include <Geometry/PoseStamped.pb.h>
#include <Geometry/Twist.pb.h>
#include <Nav/Odometry.pb.h>
#include <base/BaseGeneratePath.hpp>
#include <base/BaseDwaPlanner.hpp>
#include <Std/Bool.pb.h>
#include <Std/Float64.pb.h>
#include <Nav/OccupancyGrid.pb.h>
#include <yaml-cpp/yaml.h>
#include <unistd.h>
#include <chrono>
#include <thread>

namespace DWA
{
	class BaseDwaPlanner : public Hnu::Middleware::Node
	{
	public:
		explicit BaseDwaPlanner(const std::string &name);  
		virtual ~BaseDwaPlanner();
		// main process
		virtual void plan();  /*!< 主要进程 规划全过程*/

	protected:
		std::string yaml_file_name_; /*!< yaml 文件路径*/
		// **** 来源: yaml ****
		// 话题名称们
		std::string
		goal_topic_, /*!< 目标点数组话题名称*/
		odom_topic_, /*!< 底盘速度角速度话题名称*/
		pose_topic_, /*!< 定位信息话题名称*/
		map_topic_, /*!< 局部地图话题名称*/
		cmd_vel_topic_, /*!< 发布速度话题名称*/
		replan_feedback_topic_, /*!< 重规划结果话题名称*/
		local_traj_topic_; /*!< 局部规划结果话题名称*/
		bool allow_back_;
		double control_freq_; /*!< 控制频率*/

		//创建Hmw中消息发布者和接受者
		/*!< goals订阅者*/
		std::shared_ptr<Hnu::Middleware::Subscriber<TaskPlanner::GlobalPathPlan>> goals_sub_;
		/*!< odometry 信息订阅者*/
		std::shared_ptr<Hnu::Middleware::Subscriber<Nav::Odometry>> odom_sub_;
		/*!< cmd_vel速度消息发布者*/
		std::shared_ptr<Hnu::Middleware::Publisher<Geometry::Twist>> cmd_vel_pub_;
		/*!< 重规划消息发布者*/
		std::shared_ptr<Hnu::Middleware::Publisher<TaskPlanner::GlobalPathPlanFeedback>> replan_pub_;
		/*!< 当前位置消息订阅者*/
		std::shared_ptr<Hnu::Middleware::Subscriber<Geometry::PoseStamped>> current_pose_sub_;
		/*!< 最大速度消息订阅者*/
		std::shared_ptr<Hnu::Middleware::Subscriber<Std::Float64>> max_vel_sub_;
		/*!<  重规划消息订阅者*/
		std::shared_ptr<Hnu::Middleware::Subscriber<Std::Bool>> replan_msg_sub_;
		/*!< 局部规划轨迹发布者*/
		// rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr traj_pub_;

		//消息内容的本地缓存
		std::vector<Geometry::Pose> points_; /*!< 目标点的本地存储数组*/
		std::shared_ptr<Geometry::PoseStamped> current_pose_; /*!< 当前定位信息*/
		std::shared_ptr<Nav::Odometry> odom_; /*!< odometry消息的本地存储*/
		std::shared_ptr<Nav::OccupancyGrid> costmap_; /*!< 局部地图消息的本地存储*/

		//用于确认各消息状态 待均就绪才能启动规划
		volatile bool
		odom_update_, /*!< 底盘速度角速度 话题就绪情况*/
		goal_update_, /*!< 目标点数组 话题就绪情况*/
		pose_update_, /*!< 定位 话题就绪情况*/
		map_update_; /*!< 局部地图 话题就绪情况*/

		int current_step_; /*!< 当前局部规划目标点index*/
		double goal_tolerance_; /*!<  容忍度 对于目标抵达的判断*/
		double max_vel_from_global_; /*!<  x轴最大速度 来源于全局*/
		BaseGeneratePath *GP; /*!<  生成路径类 每个planner初始化一个生成路径类*/

		std::shared_ptr<std::thread> process_thread_ = NULL;         /*!<  dwa planner 规划线程*/
		Eigen::Vector3d last_goal_, /*!< 上一个目标点*/ local_goal_; /*!< 当前局部目标点*/

		virtual void goalCallback(std::shared_ptr<TaskPlanner::GlobalPathPlan> msg);  /*!< 接收到目标点数组后的回调函数*/
		void odomCallback(std::shared_ptr<Nav::Odometry> odom);                       /*!< 接收到 底盘速度  消息后的回调函数*/
		void poseCallback(std::shared_ptr<Geometry::PoseStamped> pose);               /*!< 接收到 定位  消息后的回调函数*/
		void maxVelCallback(std::shared_ptr<Std::Float64> msg);                       /*!< 接收到远程发来的最大速度控制消息后的回调函数*/
		void replanCallback(std::shared_ptr<Std::Bool> msg);                          /*!< 接收到a_server  的重规划消息后的回调函数*/
		void mapCallback(std::shared_ptr<Nav::OccupancyGrid> msg);                   /*!< 接收到 局部地图  消息后的回调函数*/
		double getYaw(const Geometry::Quaternion& q); 																	 /*!< 获取四元数的yaw角*/
		bool readYaml();                                                                    /*!< yaml 读取函数*/
		virtual bool goalReached();                                                         /*!< 判断是否到达目标点*/
		virtual bool getLocalGoal();                                                        /*!< local goal 更新函数 本版本为寻点*/
		virtual std::vector<std::vector<float>> raycast();                                  /*!<  获取锥形/扇形区域内障碍物坐标 ,本版本为扇形区域*/

		std::shared_ptr<Hnu::Middleware::Timer> timer_;
		int watch_cnt[6] = {1000, 1000, 1000, 1000, 1000, 1000};
		void on_timer();
	};
}
