//
// Created by aoao on 25-4-7.
//

#pragma once

#include "MotionState.hpp"
#include <yaml-cpp/yaml.h>
#include "Geometry/PoseStamped.pb.h"
#include "ApeLine.hpp"
#include <spdlog/spdlog.h>

namespace DWA
{
    class BaseScore
    {
    public:
        BaseScore(const std::string filename, const Eigen::Vector3d last_goal, const Eigen::Vector3d local_goal,const State current, const std::vector<std::vector<float>> *obs_list = NULL);
        virtual ~BaseScore();
        virtual float calculateCost(std::vector<std::shared_ptr<State>> Traj);

    protected:
        // 四个代价计算的函数
        virtual void calcGoalCost();
        void calcSpeedCost();
        virtual void calcPathCost();
        virtual void calcObsCost();
        void readYaml(const std::string filename);

    protected:
        std::vector<std::shared_ptr<State>> traj_;/*!< 待评估轨迹*/
        std::vector<std::array<double, 2>> footprint_;/*!< 车身信息*/
        const std::vector<std::vector<float>> *obs_list_;/*!< 障碍物*/
        // 权重
        float path_bias_,/*!< 走直线 权重*/ goal_bias_,/*!< 前往目标点 权重*/ speed_bias_,/*!< 速度 权重*/ obs_bias_;/*!< 避障 权重*/
        // 代价
        float goal_cost_,/*!< 前往目标点 代价*/ path_cost_,/*!< 走直线 代价*/ speed_cost_,/*!< 速度 代价*/ obs_cost_;/*!< 避障 代价*/
        Eigen::Vector3d local_goal_;/*!< 局部目标点*/
        float min_obs_dist_,/*!< 膨胀后与障碍物最小距离*/ thresh_vel;/*!< 阈值 速度超过阈值需要二次膨胀*/
        double target_velocity_;/*!< 期望速度*/
        apeLine *line_;/*!< 直线( last goal ->local goal) */


    private:

    };
}

