//
// Created by aoao on 25-4-8.
//

#include "base/BaseDwaPlanner.hpp"
using namespace DWA;

namespace DWA
{
	BaseDwaPlanner::BaseDwaPlanner(const std::string &name) : Node(name)
	{
		std::string yamlPath;
		char buffer[1024];
		ssize_t count = readlink("/proc/self/exe", buffer, 1024);
		if (count != -1) {
			buffer[count] = '\0';
      yamlPath = std::string(dirname(buffer)) + "/LocalPlannerConfig/BaseParam.yaml";
  	}

		yaml_file_name_ = yamlPath;
		// 初始化参数
		readYaml();
		current_step_ = 0;
		GP = new BaseGeneratePath(yamlPath);
		max_vel_from_global_ = 0.0;
		odom_update_ = false;
		map_update_ = false;
		goal_update_ = false;
		pose_update_ = false;
		current_pose_ = std::make_shared<Geometry::PoseStamped>();
		// goal_ = std::make_shared<geometry_msgs::msg::PoseStamped>();
		odom_ = std::make_shared<Nav::Odometry>();
		current_pose_sub_ = createSubscriber<Geometry::PoseStamped>(pose_topic_,std::bind(&BaseDwaPlanner::poseCallback, this, std::placeholders::_1));
		max_vel_sub_= createSubscriber<Std::Float64>("/max_vel_adjust", std::bind(&BaseDwaPlanner::maxVelCallback, this, std::placeholders::_1));
		replan_msg_sub_ = createSubscriber<Std::Bool>("/a_server_replan",std::bind(&BaseDwaPlanner::replanCallback, this, std::placeholders::_1));
		goals_sub_ = createSubscriber<TaskPlanner::GlobalPathPlan>(goal_topic_, std::bind(&BaseDwaPlanner::goalCallback,this,std::placeholders::_1));
		odom_sub_ = createSubscriber<Nav::Odometry>(odom_topic_, std::bind(&BaseDwaPlanner::odomCallback, this,std::placeholders::_1));
		cmd_vel_pub_ = this->createPublisher<Geometry::Twist>(cmd_vel_topic_);
		// traj_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(local_traj_topic_, 1);
		replan_pub_ = this->createPublisher<TaskPlanner::GlobalPathPlanFeedback>(replan_feedback_topic_);
		std::cout << " finish BaseDwaPlanner constructor" << std::endl;
		timer_ = createTimer(50, std::bind(&BaseDwaPlanner::on_timer, this));
	}

	//看门狗机制。20个interval内没有收到消息，就认为消息未更新。
	void BaseDwaPlanner::on_timer()
	{
		constexpr uint32_t cnt_max = 20;
		for (size_t i = 0; i < 4; i++)
		{
			if(watch_cnt[i] < cnt_max) {
				switch (i)
				{
				case 0:
					odom_update_ = true;
					break;
				case 2:
					pose_update_ = true;
					break;
				case 3:
					map_update_ = true;
					break;
				default: ;
				}
			}
			else {  //1秒未收到twist喂狗信息，就停掉
				switch (i)
				{
				case 0:
					odom_update_ = false;
					break;
				case 2:
					pose_update_ = false;
					break;
				case 3:
					map_update_ = false;
					break;
				default: ;
				}
			}
		}
		for (size_t i = 0; i < 4; i++)
		{
			watch_cnt[i]++;
			if(watch_cnt[i] > 1000000) {
				watch_cnt[i] = cnt_max + 1;
			}
		}
	}

	bool BaseDwaPlanner::readYaml()
	{
		try
		{
			std::cout << "Actual param file: " << yaml_file_name_ << std::endl;
			// 获取配置项
			YAML::Node config = YAML::LoadFile(yaml_file_name_);
			goal_topic_ = config["topic"]["goal_topic"].as<std::string>(); //目标话题信息
			odom_topic_ = config["topic"]["odom_topic"].as<std::string>(); //里程计信息话题
			pose_topic_ = config["topic"]["pose_topic"].as<std::string>(); //位姿信息话题
			map_topic_ = config["topic"]["map_topic"].as<std::string>();	 //地图信息话题
			cmd_vel_topic_ = config["topic"]["speed_topic"].as<std::string>(); //速度命令话题
			replan_feedback_topic_ = config["topic"]["feedback_topic"].as<std::string>(); //重规划反馈话题
			local_traj_topic_ = config["topic"]["local_traj_topic"].as<std::string>(); //局部规划结果话题
			goal_tolerance_ = config["dwa"]["goal_tolerance_"].as<double>(); //目标点容忍度
			allow_back_ = config["kinematic"]["allow_back"].as<bool>(); //是否允许后退
		}
		catch (const YAML::Exception &ex)
		{
			// 如果出现异常，则意味着读取和解析YAML文件失败
			std::cerr << "YAML file reading or parsing failed in DWAPLANNER.CPP. Error message: " << ex.what() << std::endl;

			goal_tolerance_ = 1.0;
			odom_topic_ = "/odom";
			goal_topic_ = "/global_path";
			cmd_vel_topic_ = "/cmd_vel";
			map_topic_ = "/gridMap";
			replan_feedback_topic_ = "/global_path_plan_feedback";
			local_traj_topic_ = "local_traj";
			allow_back_ = false;
		}
		return true;
	}

	BaseDwaPlanner::~BaseDwaPlanner()
	{
		delete GP;
		std::cout << " finish BaseDwaPlanner destructor" << std::endl;
	}

	void BaseDwaPlanner::poseCallback(std::shared_ptr<Geometry::PoseStamped> pose)
	{
		current_pose_ = std::move(pose);
	}

	void BaseDwaPlanner::goalCallback(const std::shared_ptr<TaskPlanner::GlobalPathPlan> msg)
	{
		//goal_update_ = false;
		// points_ = msg->pose_array().poses;
		for (const auto& pose : msg->pose_array().poses()) {
			points_.push_back(pose);  // 可能还需要手动转换类型
		}
		// 是否允许后退
		allow_back_ = msg->back_up();
		// max_vel_from_global_ = msg->velocity;
		if (allow_back_)
		{
			spdlog::debug("Attention! Enable back\n");
		}
		spdlog::debug("get goal {}", points_.size());
		if(pose_update_) {
			current_step_ = 0;
			last_goal_ = {current_pose_->pose().position().x(), current_pose_->pose().position().y(), getYaw(current_pose_->pose().orientation())};
			local_goal_ = {points_[current_step_].position().x(), points_[current_step_].position().y(), getYaw(points_[current_step_].orientation())};
			spdlog::debug("Update last goal{}{}{}", current_step_, last_goal_[0], last_goal_[1]);
			spdlog::debug("Update local goal {}{}{}", current_step_, local_goal_[0], local_goal_[1]);
			// usleep(5000*1000);
			goal_update_ = true;
		}
	}

	void BaseDwaPlanner::mapCallback(const std::shared_ptr<Nav::OccupancyGrid> msg)
	{
		costmap_ = std::move(msg);
		// 获取costmap里的位置（来源：组合导航）
		// ps : 目前的当前位置的朝向来源为组合导航～
		// current_pose_->header.frame_id = costmap_->header.frame_id;
		//  current_pose_->pose.position = costmap_->info.origin.position;
		// current_pose_->pose.orientation = costmap_->info.origin.orientation;
		watch_cnt[3] = 0;
	}

	void BaseDwaPlanner::odomCallback(const std::shared_ptr<Nav::Odometry> odom)
	{
		watch_cnt[0] = 0;
		odom_ = odom;
		// ? odometry 坐标系是啥
		odom_->mutable_header()->set_frame_id("odom");

		// 如果需要讲当前位置的朝向来源调整为odom 需要在这里做修改 并且需要修改frame ID？我猜的 还没验证
		//  current_pose_->pose.orientation = odom_->pose.pose.orientation;

	}

	void BaseDwaPlanner::maxVelCallback(const std::shared_ptr<Std::Float64> msg)
	{
		max_vel_from_global_ = msg->data();
		std::cout<<"judge max_vel_ to "<<max_vel_from_global_<<std::endl;
	}

	void BaseDwaPlanner::replanCallback(const std::shared_ptr<Std::Bool> msg)
	{
		std::cout<<"replan occur"<<std::endl;
		Geometry::Twist cmd_vel;
		cmd_vel.mutable_linear()->set_x(0);
		cmd_vel.mutable_linear()->set_y(0);
		cmd_vel.mutable_angular()->set_z(0);
		cmd_vel_pub_->publish(cmd_vel);
		goal_update_ = false;
	}

	bool BaseDwaPlanner::goalReached()
	{
		if (current_step_ >= (points_.size() - 1))
		{
			if ((hypot(local_goal_[0] - current_pose_->pose().position().x(), local_goal_[1] - current_pose_->pose().position().y()) < goal_tolerance_))
			{
				spdlog::debug("arrive global goal {}", current_step_);
				return true;
			}
		}
		else if ((hypot(local_goal_[0] - current_pose_->pose().position().x(), local_goal_[1] - current_pose_->pose().position().y()) < 1.5 * goal_tolerance_))
		{
			spdlog::debug("arrive global goal {}", current_step_);
			goal_update_ = false;
			getLocalGoal();
			goal_update_ = true;
		}
		return false;
	}

	bool BaseDwaPlanner::getLocalGoal()
	{
		last_goal_ = {points_[current_step_].position().x(), points_[current_step_].position().y(), getYaw(points_[current_step_].orientation())};

		current_step_++;
		if (current_step_ == points_.size())
			return false;
		// goal_->pose = points_[current_step_];
		local_goal_ = {points_[current_step_].position().x(), points_[current_step_].position().y(), getYaw(points_[current_step_].orientation())};
		spdlog::debug("Update last goal {},{},{}", current_step_, last_goal_[0], last_goal_[1]);
		spdlog::debug("Update local goal {},{},{}", current_step_, local_goal_[0], local_goal_[1]);
		return true;
	}

	std::vector<std::vector<float>> BaseDwaPlanner::raycast()
	{
		std::vector<std::vector<float>> obs_list;
		return obs_list;
	}

	void BaseDwaPlanner::plan()
	{
    int current_temp;
    auto thread_start = std::chrono::steady_clock::now(); // 获取系统时间
    current_temp = current_step_ - 1;
    while (true)
    {
      if (goal_update_ && odom_update_ && pose_update_ && map_update_)
      {
       auto single_start = std::chrono::steady_clock::now(); // 获取系统时间

				spdlog::debug("===dwa planner=== ");
				spdlog::debug("current pose : {},{}", current_pose_->pose().position().x(), current_pose_->pose().position().y());
				spdlog::debug("current goal : {},{}", local_goal_[0], local_goal_[1]);

        if (goalReached())
        {
          Geometry::Twist cmd_vel;
          cmd_vel.mutable_linear()->set_x(0);
          cmd_vel.mutable_linear()->set_y(0);
          cmd_vel.mutable_angular()->set_z(0);
          cmd_vel_pub_->publish(cmd_vel);
          spdlog::debug("Goal Reached");
          goal_update_ = false;
					// odom_update_ = pose_update_ = false;
          usleep(500);
          continue;
          // break;
        }
        // 判断是否到达局部目标点 更新时间
        // 如果重规划或者收到新的目标点也会重新更新时间
        if (current_temp != current_step_)
        {
            current_temp = current_step_;
            thread_start = std::chrono::steady_clock::now(); // 获取系统时间
        }

        std::vector<std::vector<float>> obs_list = raycast();

        State current(current_pose_->pose().position().x(), current_pose_->pose().position().y(),
                      getYaw(current_pose_->pose().orientation()), odom_->twist().twist().linear().x(),
                      odom_->twist().twist().angular().z());
        std::vector<std::shared_ptr<State>> traj;
        if (obs_list.size() == 0)
            traj = GP->generateTrajectory(current, last_goal_, local_goal_, allow_back_, max_vel_from_global_);
        else
            traj = GP->generateTrajectory(current, last_goal_, local_goal_, allow_back_,  max_vel_from_global_, &obs_list);
        Geometry::Twist cmd_vel;
        cmd_vel.mutable_linear()->set_x(traj[0]->velocity);
        cmd_vel.mutable_linear()->set_y(0);
        cmd_vel.mutable_angular()->set_z(traj[0]->yawrate);
        std::cout << " publish cmd_vel(x,yaw_z): (" << cmd_vel.linear().x() << ',' << cmd_vel.angular().z() << ')'
                  << std::endl;
        cmd_vel_pub_->publish(cmd_vel);

        //odom_update_ = pose_update_ = false;

        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> time_taken = end - single_start;
        if (time_taken.count() > (1.0 / control_freq_))
          spdlog::warn("Control loop failed. Desired frequency is {}Hz. The loop actually took {} seconds",
                      control_freq_, time_taken.count());

        // 本次局部目标点规划时间超过2分钟 则启动重规划
        /*
                        time_taken = end - thread_start;
                        if (time_taken.seconds() > 300.0)
                        {
                            RCLCPP_INFO(get_logger(), "Occur replan after %f seconds", time_taken.seconds());
                            task_planner_msgs::msg::GlobalPathPlanFeedbackMsg impassable;
                            if (current_step_ >= 1)
                            {
                                impassable.pose_array.poses.push_back(points_[current_step_ - 1]);
                                impassable.pose_array.poses.push_back(points_[current_step_]);
                            }
                            else //按照全局部分重规划的要求 不能发(0,1) 因为第一个点（0） 不在路网上
                            {
                                impassable.pose_array.poses.push_back(points_[0]);
                                impassable.pose_array.poses.push_back(points_[1]);
                            }
                            std::cout << "Publish replan msg: 0: " << impassable.pose_array.poses[0].position.x << ' ' << impassable.pose_array.poses[0].position.y << " 1: " << impassable.pose_array.poses[1].position.x << ' ' << impassable.pose_array.poses[1].position.y << std::endl;
                            replan_pub_->publish(impassable);
                            // replan 需要等待发布新的目标点
                            goal_update_ = false;
                            usleep(200);
                        }
        */
      }
        // else {   usleep(20);std::cout<<odom_update_<<' '<<map_update_<<' '<<pose_update_<<" "<<goal_update_<<std::endl;}
        usleep(20);
		}
	}
	double BaseDwaPlanner::getYaw(const Geometry::Quaternion& q)
	{
		// 提取四元数的分量
		double yaw;
		double sqw;
		double sqx;
		double sqy;
		double sqz;

		sqx = q.x() * q.x();
		sqy = q.y() * q.y();
		sqz = q.z() * q.z();
		sqw = q.w() * q.w();

		double sarg = -2 * (q.x() * q.z() - q.w() * q.y()) / (sqx + sqy + sqz + sqw);

		if (sarg <= -0.99999) {
			yaw = -2 * atan2(q.y(), q.x());
		} else if (sarg >= 0.99999) {
			yaw = 2 * atan2(q.y(), q.x());
		} else {
			yaw = atan2(2 * (q.x() * q.y() + q.w() * q.z()), sqw + sqx - sqy - sqz);
		}
		return yaw;
	}
}
