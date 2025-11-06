//
// Created by aoao on 25-4-7.
//

#pragma once
#include <iostream>
#include <cmath>
namespace DWA
{
    class State
    {
    public:
        State(double X, double Y, double YAW, double VELOCITY, double YAWRATE)
          : x(X), y(Y), yaw(YAW), velocity(VELOCITY), yawrate(YAWRATE)
        {

        }
        State()
        {
            x = y = yaw = velocity = yawrate = 0;
        }

        virtual void state_motion(const double Velocity, const double YawRate, const double DT, double pitchRate = 0.0)
        {
            this->velocity = Velocity;
            this->yawrate = YawRate;
            this->yaw += YawRate * DT;

            this->x += Velocity * std::cos(this->yaw) * DT;
            this->y += Velocity * std::sin(this->yaw) * DT;
        }
        State(const State &a)
        {
            this->x = a.x;
            this->y = a.y;
            this->yaw = a.yaw;
            this->velocity = a.velocity;
            this->yawrate = a.yawrate;
        }
        double x;        // robot position x    pose_->pose.position.x,
        double y;        // robot posiiton y   pose_->pose.position.y
        double yaw;      // robot orientation yaw 围绕z轴旋转角度，也叫偏航角  tf2::getYaw(pose_->pose.orientation)
        double velocity; // robot linear velocity     odom_->twist.twist.linear.x
        double yawrate;  // robot angular velocity   odom_->twist.twist.angular.z
    };

    class State3d : public State
    {
    public:
        State3d()
        {
            z = pitch = pitchrate = 0;
        }

        State3d(double X, double Y, double Z, double YAW, double VELOCITY, double YAWRATE, double PITCH, double PITCHRATE)
        {
            x = X;
            y = Y;
            z = Z;
            yaw = YAW;
            velocity = VELOCITY;
            yawrate = YAWRATE;
            pitch = PITCH;
            pitchrate = PITCHRATE;
        }
        State3d(const State3d &a)
        {
            this->x = a.x;
            this->y = a.y;
            this->yaw = a.yaw;
            this->velocity = a.velocity;
            this->yawrate = a.yawrate;
            this->pitch = a.pitch;
            this->z = a.z;
            this->pitchrate = a.pitchrate;
        }
        double z;         // robot position z
        double pitch;     // robot orientation pitch 俯仰角
        double pitchrate; // robot angula r velocity   odom_->twist.twist.angular.y
        void state_motion(const double Velocity, const double YawRate, const double DT, double PitchRate = 0.0)
        {
            std::cout << "3d state motion \n";
            this->velocity = velocity;
            this->yawrate = YawRate;
            this->pitchrate = PitchRate;
            this->yaw += YawRate * DT;
            this->pitch += PitchRate * DT;
            this->x += Velocity * std::cos(this->yaw) * std::cos(this->pitch) * DT;
            this->y += Velocity * std::sin(this->yaw) * std::cos(this->pitch) * DT;
            this->z += Velocity * std::sin(this->pitch) * DT;
        }
    };
}


