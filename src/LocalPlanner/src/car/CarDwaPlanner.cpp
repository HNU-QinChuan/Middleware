#include "car/CarDwaPlanner.hpp"

namespace DWA
{
  CarDwaPlanner::CarDwaPlanner(const std::string &name) : BaseDwaPlanner(name)
  {
    // car 无需接收pose map包含pos
    grid_map_sub_ = this->createSubscriber<Nav::OccupancyGrid>(
      map_topic_, std::bind(&CarDwaPlanner::mapCallback, this, std::placeholders::_1));
    process_thread_ = std::make_shared<std::thread>(std::bind(&CarDwaPlanner::plan, this));
    replan_pub_ = this->createPublisher<TaskPlanner::GlobalPathPlanFeedback>(replan_feedback_topic_);
    spdlog::debug("finish CarDwaPlanner constructor");
  }

  CarDwaPlanner::~CarDwaPlanner()
  {
    spdlog::debug("finish car destructor");
  }
  // local goal 更新 ,本版本为前看
  // 9-17  取消前看

  bool CarDwaPlanner::getLocalGoal()
  {
    if (current_step_ == points_.size())
      return false;
    int i;

    // 与ship planner 不同，前看 3个目标点
    int forward = current_step_ + 3;
    // 防止越界
    if (points_.size() < forward)
      forward = points_.size();

    float semi_costmap_width = 0.5 * costmap_->info().width() * costmap_->info().resolution();
    float semi_costmap_height = 0.5 * costmap_->info().height() * costmap_->info().resolution();
    for (i = current_step_; i < forward; i++)
    {
      if ((std::abs(current_pose_->pose().position().x() - points_[i].position().x()) > semi_costmap_height) || (std::abs(current_pose_->pose().position().y() - points_[i].position().y()) > semi_costmap_width))
        break;
    }
    if (i == forward)
      current_step_ = i - 1;
    else
      current_step_ = i;

    if (current_step_ == points_.size())
      return false;

    // goal_->pose = points_[current_step_];
    last_goal_ = {current_pose_->pose().position().x(), current_pose_->pose().position().y(), getYaw(current_pose_->pose().orientation())};
    local_goal_ = {points_[current_step_].position().x(), points_[current_step_].position().y(), getYaw(points_[current_step_].orientation())};

    spdlog::debug("Update last goal {},{},{}", current_step_, last_goal_[0], last_goal_[1]);
    spdlog::debug("Update local goal {},{},{}", current_step_, local_goal_[0], local_goal_[1]);
    return true;
  }
  //     void CarDwaPlanner::map_callback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
  //     {
  //         costmap_ = msg;
  //         // 获取costmap里的位置（来源：组合导航）
  //         // ps : 目前的当前位置的朝向来源为组合导航～
  //         current_pose_->header.frame_id = costmap_->header.frame_id;
  //         current_pose_->pose.position = costmap_->info.origin.position;
  // //        current_pose_->pose.orientation = costmap_->info.origin.orientation;
  //         map_update_ = true;
  //     }
  std::vector<std::vector<float>> CarDwaPlanner::raycast()
  {
    std::vector<std::vector<float>> obs_list;
    double current_yaw = getYaw(current_pose_->pose().orientation());
    double resolution = costmap_->info().resolution();
    double map_width = costmap_->info().width();
    double map_height = costmap_->info().height();
    double MAX_DIST = map_width * resolution;
    float current_pose_x = current_pose_->pose().position().x();
    float current_pose_y = current_pose_->pose().position().y();

    for (float angle = current_yaw - M_PI; angle <= current_yaw + M_PI; angle += 0.2)
    {
      for (float dist = 0.0; dist <= MAX_DIST; dist += resolution)
      {
        float x = dist * cos(angle);
        float y = dist * sin(angle);

        int i = floor(x / resolution + 0.5) + map_width * 0.5;
        int j = floor(y / resolution + 0.5) + map_height * 0.5;
        if ((i < 0 || i >= map_width) || (j < 0 || j >= map_height))
        {
          break;
        }
        //   need test
        // 我咋觉得要用height 算了，时间来不及了，不敢改 先放着吧
        if (costmap_->data(j * map_width + i) == 100)
        {
          // std::cout<<"obstacle (X,Y) :"<<x<<","<<y<<'\n';
          std::vector<float> obs_state = {x + current_pose_x, y + current_pose_y};
          obs_list.push_back(obs_state);
          break;
        }
      }
    }
    return obs_list;
  }


}