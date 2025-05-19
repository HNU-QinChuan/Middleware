//
// Created by aoao on 25-4-8.
//

#include "base/BaseGeneratePath.hpp"

namespace DWA
{
// @brief 读取生成路径的限制 （速度限制 加速度限制等）
// @param filename 读取配置文件路径
	void BaseGeneratePath::readYaml(std::string filename)
	{
		yaml_file_path_ = filename;
		try
		{
			YAML::Node config = YAML::LoadFile(filename);
			max_vel_x_ = config["kinematic"]["max_vel"]["max_vel_x"].as<double>();
			min_vel_x_ = config["kinematic"]["min_vel"]["min_vel_x"].as<double>();
			max_vel_y_ = config["kinematic"]["max_vel"]["max_vel_y"].as<double>();
			min_vel_y_ = config["kinematic"]["min_vel"]["min_vel_y"].as<double>();
			acc_lim_x_ = config["kinematic"]["max_acc"]["max_acc_x"].as<double>();
			acc_lim_y_ = config["kinematic"]["max_acc"]["max_acc_y"].as<double>();
			acc_lim_theta_ = config["kinematic"]["acc_w"].as<double>();
			deacc_lim_theta_ = config["kinematic"]["deacc_w"].as<double>();
			deacc_lim_x_ = config["kinematic"]["deacc_v"]["deacc_v_x"].as<double>();
			deacc_lim_y_ = config["kinematic"]["deacc_v"]["deacc_v_y"].as<double>();

			min_vel_theta_ = config["kinematic"]["min_w"].as<double>();
			max_vel_theta_ = config["kinematic"]["max_w"].as<double>();
			num_samples_x_ = config["simulate"]["num_vel"].as<int>();
			num_samples_theta_ = config["simulate"]["num_w"].as<int>();

			num_steps_ = config["simulate"]["num_steps"].as<int>();
			sim_time_ = config["simulate"]["sim_time"].as<double>();
			allow_pub_ = config["simulate"]["allow_pub"].as<bool>();
		}
		catch (const YAML::Exception &ex)
		{
			spdlog::warn("error while reading para file in BaseGeneratePath.cpp");
			max_vel_x_ = 1;
			min_vel_x_ = 0;
			max_vel_y_ = 0;
			min_vel_y_ = 0;
			acc_lim_x_ = 0.5;
			acc_lim_y_ = 0;
			acc_lim_theta_ = 0.25;
			deacc_lim_theta_ = -0.25;
			deacc_lim_x_ = -0.5;
			deacc_lim_y_ = 0;

			min_vel_theta_ = -1;
			max_vel_theta_ = 1;
			num_samples_x_ = 30;
			num_samples_theta_ = 20;

			num_steps_ = 30;
			sim_time_ = 4.0;
			allow_pub_ = false;
		}
	}

	BaseGeneratePath::BaseGeneratePath(std::string filename)
	{
		readYaml(filename);
		spdlog::debug("finish BaseGeneratePath construtor");
	}

	BaseGeneratePath::~BaseGeneratePath()
	{
		spdlog::debug("finish BaseGeneratePath destructor");
	}
	/// @brief 获取score类 主要是三维和二维的评价函数不一样
	/// @param current 当前位置（x,y,yaw) 或 （ X,  Y,  Z,  YAW,  VELOCITY,  YAWRATE,  PITCH,  PITCHRATE)
	/// @param last_goal  上一个目标点 （x,y,yaw) 或 三维(x,y,z)
	/// @param local_goal 当前目标点   （x,y,yaw) 或 三维(x,y,z)
	/// @param obs_list 障碍物集合
	/// @return score类类型 1:base 2:missile
	int BaseGeneratePath::getScoreClass(const State current, const Eigen::Vector3d last_goal,
		const Eigen::Vector3d local_goal, const std::vector<std::vector<float>> *obs_list)
	{
		sc_ = new BaseScore(yaml_file_path_, last_goal, local_goal, current, obs_list);
		spdlog::debug("finish getScoreClass");
		return 1;
	}
	/// @brief 生成路径
	/// @param current 当前位置（x,y,yaw) 或 （ X,  Y,  Z,  YAW,  VELOCITY,  YAWRATE,  PITCH,  PITCHRATE)
	/// @param last_goal  上一个目标点 （x,y,yaw) 或 三维(x,y,z)
	/// @param local_goal 当前目标点   （x,y,yaw) 或 三维(x,y,z)
	/// @param allow_back 是否允许后退
	/// @param traj_pub_ 局部规划结果发布者指针
	/// @param obs_list 障碍物集合
	/// @return 生成的最佳路径

	std::vector<std::shared_ptr<State>> BaseGeneratePath::generateTrajectory
	(
		const State current, Eigen::Vector3d last_goal,
		Eigen::Vector3d local_goal, bool allow_back,
		// rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr traj_pub_,
		double max_velocity, std::vector<std::vector<float>> *obs_list
	)
	{
		double vel_x = current.velocity;
		double vel_theta = current.yawrate;

		double dt = sim_time_ / num_steps_;
		double max_vel_x = std::min(max_vel_x_, vel_x + acc_lim_x_ * sim_time_);
		// max_velocity 来源于指控
		if (std::abs(max_velocity) >= 1e-8)
		{
			max_vel_x = std::min(max_vel_x, max_velocity);
			// 为了调试 之后删除
			spdlog::debug("judge ok!");
		}
		double max_vel_theta = std::min(max_vel_theta_, vel_theta + acc_lim_theta_ * sim_time_);
		double min_vel_x = std::min(std::max(min_vel_x_, vel_x + deacc_lim_x_ * sim_time_), max_vel_x);
		if (!allow_back)
			min_vel_x = std::max(min_vel_x, 0.0);
		double min_vel_theta = std::min(std::max(min_vel_theta_, vel_theta + deacc_lim_theta_ * sim_time_), max_vel_theta);
		// 最小0.1
		double step_size_x = std::max((max_vel_x - min_vel_x) / (std::max(1, (num_samples_x_ - 1))), 0.1);
		// 最小0.05
		double step_size_theta = std::max((max_vel_theta - min_vel_theta) / (std::max(1, (num_samples_theta_ - 1))), 0.05);
		float min_cost = 1e6;
		std::vector<std::shared_ptr<State>> best_traj;
		// visualization_msgs::msg::MarkerArray mark_trajs;
		// visualization_msgs::msg::Marker mark;
		int best_traj_index = 0;
		std::vector<std::shared_ptr<State>> traj;
		getScoreClass(current, last_goal, local_goal, obs_list);
		// if (allow_pub_)
		// {
		// 	mark.header.frame_id = "map";
		// 	mark.ns = "markers";
		// 	mark.type = visualization_msgs::msg::Marker::LINE_STRIP;
		// 	mark.action = visualization_msgs::msg::Marker::ADD;
		// 	mark.scale.x = 0.1;
		// 	mark.pose.orientation.w = 1.0;
		// 	mark.scale.y = 0.1;
		// 	mark.color.a = 1.0; // 透明度
		// 	mark.color.r = 0.0;
		// 	mark.color.g = 1.0;
		// 	mark.color.b = 1.0;
		// }
		int x = 0;
    for (double v = min_vel_x; v <= max_vel_x; v += step_size_x)
    {
        for (double k = min_vel_theta; k <= max_vel_theta; k += step_size_theta)
        {
            x++;

            // 发布轨迹
            if (allow_pub_)
            {
                // mark.header.stamp = rclcpp::Time();
                //
                // mark.id = mark_trajs.markers.size();
                //
                // std::shared_ptr<State> temp_state(new State(current));
                // std::vector<std::shared_ptr<State>> traj;
                //
                // mark.points.empty();
                // mark.points.erase(mark.points.begin(), mark.points.begin() + mark.points.size());
                //
                // for (int i = 0; i < num_steps_; i++)
                // {
                //     temp_state->state_motion(v, k, dt);
                //     traj.push_back(temp_state);
                //     if (i % 5 == 0)
                //     {
                //         geometry_msgs::msg::Point p;
                //         p.x = temp_state->x;
                //         p.y = temp_state->y;
                //         p.z = 0;
                //         mark.points.push_back(p);
                //     }
                // }
                //
                // // mark_trajs.markers[x].color.g = (float)x/(float)step_size_x;
                // //  x++;
                // float final_cost = sc_->calculateCost(traj);
                // if (final_cost == -1)
                //     continue;
                // if (x % 5 == 0)
                //     mark.color.a = 0;
                // else
                //     mark.color.a = 1.0;
                // mark_trajs.markers.push_back(mark);
                // if (final_cost <= min_cost)
                // {
                //     min_cost = final_cost;
                //     best_traj = traj;
                //     best_traj_index = mark_trajs.markers.size();
                // }
            }
            // 不发布轨迹
            else
            {

                std::shared_ptr<State> temp_state(new State(current));
                std::vector<std::shared_ptr<State>> traj;

                for (int i = 0; i < num_steps_; i++)
                {
                    temp_state->state_motion(v, k, dt);
                    traj.push_back(temp_state);
                }

                // mark_trajs.markers[x].color.g = (float)x/(float)step_size_x;
                //  x++;
                float final_cost = sc_->calculateCost(traj);
                if (final_cost == -1)
                    continue;

                if (final_cost <= min_cost)
                {
                    min_cost = final_cost;
                    best_traj = traj;
                    best_traj_index = x;
                }
            }
        }
    }
    delete sc_;
    if (min_cost == 1e6)
    {
        std::shared_ptr<State> temp(new State(current));
        temp->velocity = 0.0;
        temp->yawrate = 0.0;
				spdlog::debug("could not generate a valid path");
        best_traj.push_back(temp);
    }
    // if (allow_pub_)
    // {
    //     mark_trajs.markers[best_traj_index].color.r = 1.0;
    //     mark_trajs.markers[best_traj_index].color.g = 0.0;
    //     mark_trajs.markers[best_traj_index].color.b = 0.0;
    //     mark_trajs.markers[best_traj_index].color.a = 1.0;
    //     mark_trajs.markers[best_traj_index].scale.x = 0.2;
    //     mark_trajs.markers[best_traj_index].scale.y = 0.2;
    //     mark_trajs.markers[best_traj_index].header.stamp = rclcpp::Time();
    //     traj_pub_->publish(mark_trajs);
    // }
    return best_traj;
	}
}
