#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/common/common.hh>
#include <ignition/math/Pose3.hh>
#include <memory>

#include "Geometry/PoseStamped.pb.h"
#include "hmw/Node.hpp"
#include "hmw/Publisher.hpp"


namespace gazebo
{
  class PosePublisherPlugin : public gazebo::ModelPlugin
  {
    public:
      void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf)
      {
        gzmsg << "[PosePublisherPlugin] 插件已加载成功。" << std::endl;
        this->model_ = _model;
        this->world_ = _model->GetWorld();
        last_publish_time_ = world_->SimTime();

        this->node_ = std::make_shared<Hnu::Middleware::Node>("gazebo_pose_publisher_plugin_node");
        this->pose_pub_ = node_->createPublisher<Geometry::PoseStamped>("pose");

        this->link_ = model_->GetLink("base_footprint");

        update_connection_ = event::Events::ConnectWorldUpdateBegin(
          std::bind(&PosePublisherPlugin::OnUpdate, this));

      }

      void OnUpdate()
      {
        common::Time now = world_->SimTime();
        if ((now - last_publish_time_).Double() < 0.05)  // 20 Hz
          return;
        
        last_publish_time_ = now;

        auto pose = this->link_->WorldPose();
        Geometry::PoseStamped msg;

        // msg->mutable_header()->set_stamp(now.Double());
        msg.mutable_header()->set_frame_id("map");
        msg.mutable_pose()->mutable_position()->set_x(pose.Pos().X());
        msg.mutable_pose()->mutable_position()->set_y(pose.Pos().Y());
        msg.mutable_pose()->mutable_position()->set_z(pose.Pos().Z());
        msg.mutable_pose()->mutable_orientation()->set_x(pose.Rot().X());
        msg.mutable_pose()->mutable_orientation()->set_y(pose.Rot().Y());
        msg.mutable_pose()->mutable_orientation()->set_z(pose.Rot().Z());
        msg.mutable_pose()->mutable_orientation()->set_w(pose.Rot().W());

        this->pose_pub_->publish(msg);
      }
    
    private:
      physics::ModelPtr model_;
      physics::WorldPtr world_;
      physics::LinkPtr link_;
      event::ConnectionPtr update_connection_;
      common::Time last_publish_time_;
      std::shared_ptr<Hnu::Middleware::Node> node_;
      std::shared_ptr<Hnu::Middleware::Publisher<Geometry::PoseStamped>> pose_pub_;

  };

  GZ_REGISTER_MODEL_PLUGIN(PosePublisherPlugin)
}