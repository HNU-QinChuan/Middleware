//
// Created by aoao on 25-4-7.
//

#include "base/BaseScore.hpp"

#include <utility>
namespace DWA
{
    BaseScore::BaseScore(const std::string filename, const Eigen::Vector3d last_goal, const Eigen::Vector3d local_goal, const State current, const std::vector<std::vector<float>> *obs_list)
    {
        goal_cost_ = path_cost_ = speed_cost_ = obs_cost_ = 0;
        readYaml(filename);
        this->local_goal_ = local_goal;
        this->obs_list_ = obs_list;
        const Eigen::Vector2d s1(local_goal[0], local_goal[1]);
        const Eigen::Vector2d s2(last_goal[0], last_goal[1]);
        // line_->update(s1, s2);
        this->line_ = new apeLine(s1, s2);
        std::cout << "   finish BaseScore construct" << std::endl;
    }

    void BaseScore::readYaml(const std::string filename)
    {
        float semi_width , semi_length;
        try
        {
            YAML::Node config = YAML::LoadFile(filename);
            path_bias_ = config["evaluate"]["path_bias"].as<float>();
            goal_bias_ = config["evaluate"]["goal_bias"].as<float>();
            obs_bias_ = config["evaluate"]["obs_bias"].as<float>();
            speed_bias_ = config["evaluate"]["speed_bias"].as<float>();

            semi_width = config["robot"]["width"].as<float>();
            semi_length = config["robot"]["length"].as<float>();
            semi_width = semi_width / 2;
            semi_length = semi_length / 2;

            min_obs_dist_= config["simulate"]["min_obs_dist"].as<float>();

            target_velocity_ = config["simulate"]["target_vel"].as<float>();
            thresh_vel = config["simulate"]["thresh_vel"].as<float>();
        }
        catch (const YAML::Exception &ex)
        {
            std::cout << "error while reading param file in BaseScore.cpp\n";

            goal_bias_ = path_bias_= speed_bias_= obs_bias_ = 1;
            semi_width = semi_length = 0.5;
            min_obs_dist_ = 0.2;
            target_velocity_ = 1.0;
            thresh_vel  = 2.0;

        }
        footprint_.push_back({semi_length, semi_width});
        footprint_.push_back({semi_length, 0 - semi_width});
        footprint_.push_back({semi_length, 0});
        footprint_.push_back({0 - semi_length, semi_width});
        footprint_.push_back({0 - semi_length, 0 - semi_width});
        footprint_.push_back({0 - semi_length, 0});
        footprint_.push_back({0, 0 - semi_width});
        footprint_.push_back({0, semi_width});
    }

    BaseScore::~BaseScore()
    {
        delete line_;
        //std::cout << "   finish BaseScore destructor" << std::endl;
    }

    float BaseScore::calculateCost(std::vector<std::shared_ptr<State>> Traj)
    {
        goal_cost_ = path_cost_ = speed_cost_ = obs_cost_ = 0;
        this->traj_ = std::move(Traj);
        calcGoalCost();
        calcSpeedCost();
        calcPathCost();
        if(obs_list_!=nullptr)calcObsCost();

        if ((obs_cost_ == 1e6) || (speed_cost_ == 1e6))
            return -1;
        return obs_cost_ * obs_bias_ + speed_cost_ * speed_bias_ + path_bias_ * path_cost_ + goal_bias_ * goal_cost_;
    }

    void BaseScore::calcGoalCost()
    {
        Eigen::Vector3d last_position(traj_.back()->x, traj_.back()->y, traj_.back()->yaw);
        goal_cost_ = (last_position.segment(0, 2) - local_goal_.segment(0, 2)).norm();
    }

    void BaseScore::calcSpeedCost()
    {
        // if ((traj_.back()->velocity == 0) && (traj_.back()->yawrate != 0))
        //    speed_cost_ = 1e6;
        if (traj_.back()->velocity >= 0)
            speed_cost_ = static_cast<float>(std::fabs(target_velocity_ - traj_.back()->velocity));//+std::fabs(traj_.back()->yawrate)/4;
        else
            speed_cost_ = static_cast<float>(std::fabs(traj_.back()->velocity - target_velocity_)*1.6);//+std::fabs(traj_.back()->yawrate)/4;
    }

    void BaseScore::calcObsCost()
    {
        std::cout<<"calc obs cost"<<std::endl;
        float cost = 0.0;
        float min_dist = 1e6;
        double oriented_footprint[footprint_.size()][2];

        for (const auto &state : traj_)
        {
            for (const auto &obs : *obs_list_)
            {
                double cos_th = cos(state->yaw);
                double sin_th = sin(state->yaw);
                float dist = sqrt(std::pow((state->x - obs[0]),2) + std::pow((state->y - obs[1]) ,2));
                if (dist <= min_obs_dist_)
                {
                    obs_cost_ = 1e6;
                    return ;
                }
                min_dist = std::min(min_dist, dist);
                if (state->velocity > thresh_vel)
                {
                    for (unsigned long i = 0; i < footprint_.size(); ++i)
                    {
                        oriented_footprint[i][0] = state->x + 2 * (footprint_[i][0] * cos_th - footprint_[i][1] * sin_th);
                        oriented_footprint[i][1] = state->y + 2 * (footprint_[i][0] * sin_th + footprint_[i][1] * cos_th);
                        // if (!worldToMap(oriented_footprint[i][0], oriented_footprint[i][1], x0, y0)) return false;
                        float dist = sqrt((oriented_footprint[i][0] - obs[0]) * (oriented_footprint[i][0] - obs[0]) + (oriented_footprint[i][1] - obs[1]) * (oriented_footprint[i][1] - obs[1]));
                        if (dist <= min_obs_dist_)
                        {
                            obs_cost_ = 1e6;
                            return ;
                        }
                        min_dist = std::min(min_dist, dist);
                    }
                }
                else
                {
                    for (unsigned long i = 0; i < footprint_.size(); ++i)
                    {
                        oriented_footprint[i][0] = state->x + (footprint_[i][0] * cos_th - footprint_[i][1] * sin_th);
                        oriented_footprint[i][1] = state->y + (footprint_[i][0] * sin_th + footprint_[i][1] * cos_th);
                        // if (!worldToMap(oriented_footprint[i][0], oriented_footprint[i][1], x0, y0)) return false;
                        float dist = sqrt((oriented_footprint[i][0] - obs[0]) * (oriented_footprint[i][0] - obs[0]) + (oriented_footprint[i][1] - obs[1]) * (oriented_footprint[i][1] - obs[1]));
                        if (dist <= min_obs_dist_)
                        {
                            obs_cost_ = 1e6;
                            return ;
                        }
                        min_dist = std::min(min_dist, dist);
                    }
                }
            }
        }
        obs_cost_ = 1.0 / min_dist;
        return ;
    }

    void BaseScore::calcPathCost()
    {
        Eigen::Vector2d s(traj_.back()->x, traj_.back()->y);
        path_cost_ = line_->DistanceOfPointToLine(s);
    }



}