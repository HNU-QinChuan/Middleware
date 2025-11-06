//
// Created by aoao on 25-4-8.
//

#pragma once
#include <iostream>
#include <Eigen/Eigen>
#include "MotionState.hpp"
#include <vector>
#include "base/BaseScore.hpp"
#include <hmw/Node.hpp>
#include <memory>
#include <yaml-cpp/yaml.h> //read yaml

namespace DWA
{
	class BaseGeneratePath
	{
	public:
		BaseGeneratePath(std::string filename);
		virtual ~BaseGeneratePath();
		virtual  std::vector<std::shared_ptr<State>> generateTrajectory
		(
			const State current, Eigen::Vector3d last_goal,
			Eigen::Vector3d local_goal, bool allow_back = false,
			// rclcpp::Publisher< visualization_msgs::msg::MarkerArray>::SharedPtr traj_pub_ = NULL,
			double max_velocity = 0, std::vector<std::vector<float>> *obs_list= nullptr
		);
		virtual int getScoreClass(const State current, const Eigen::Vector3d last_goal,const Eigen::Vector3d local_goal,const std::vector<std::vector<float>> *obs_list);

	private:
		bool allow_pub_;/*!< 是否允许发布局部规划结果到rviz中*/
		int num_samples_x_,/*!< 线速度 x轴方向 采样个数*/ num_samples_theta_;/*!< 偏航角速度 采样个数*/
		BaseScore *sc_; /*!< 轨迹评分类*/
		double sim_time_;/*!<  dwa生成路径时，预估多长时间后，多少个轨迹点*/
		int num_steps_;/*!<  采样点个数 num_num_steps_ * dt = sim_time_*/

		double min_vel_x_,/*!< 线速度 x轴方向 最小速度*/ min_vel_y_, /*!< 线速度 y轴方向 最小速度*/min_vel_theta_;/*!< 角速度 yaw 最小速度*/
		double max_vel_x_,/*!< 线速度 x轴方向 最大速度*/ max_vel_y_,/*!< 线速度 y轴方向 最大速度*/ max_vel_theta_;/*!< 角速度 yaw 最小速度*/
		double acc_lim_x_,/*!< x轴方向 最大加速度*/    acc_lim_y_,/*!< y轴方向 最大加速度*/ acc_lim_theta_;/*!< yaw 最大角加速度*/
		double deacc_lim_x_,/*!< x轴方向 最大减速度*/ deacc_lim_y_,/*!< y轴方向 最大减速度*/ deacc_lim_theta_;/*!< yaw 最大角减速度*/

		std::string yaml_file_path_;/*!<配置文件路径*/
		void readYaml(std::string filename);/*!< 读取配置文件*/
	};
}
