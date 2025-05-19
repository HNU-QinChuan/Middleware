#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/common/common.hh>
#include <ignition/math/Pose3.hh>

#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <Nav/Odometry.pb.h>

namespace gazebo {
  class OdomPublisherPlugin : public ModelPlugin {
  public:
    void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf) override
    {
      gzmsg << "[OdomPublisherPlugin] Plugin loaded successfully." << std::endl;
      this->model_ = _model;
      this->world_ = _model->GetWorld();
      last_publish_time_ = world_->SimTime();

      this->node_ = std::make_shared<Hnu::Middleware::Node>("gazebo_odom_publisher_plugin_node");
      this->odom_pub_ = node_->createPublisher<Nav::Odometry>("gazebo_odom_topic");

      this->link_ = model_->GetLink("base_footprint");

      update_connection_ = event::Events::ConnectWorldUpdateBegin(
        std::bind(&OdomPublisherPlugin::OnUpdate, this));
    }

    void OnUpdate()
    {
      common::Time now = world_->SimTime();
      if ((now - last_publish_time_).Double() < 0.05)  // 20 Hz
        return;
      last_publish_time_ = now;

      this->linear_vel_ = this->link_->WorldLinearVel();
      this->angular_vel_ = this->link_->WorldAngularVel();
      this->pose_ = this->link_->WorldPose();

      Nav::Odometry msg;
      // msg.mutable_header()->set_stamp(now.Double());
      msg.mutable_header()->set_frame_id("map");
      // msg.mutable_pose()->mutable_pose()->mutable_position()->set_x(pose_.Pos().X());
      // msg.mutable_pose()->mutable_pose()->mutable_position()->set_y(pose_.Pos().Y());
      // msg.mutable_pose()->mutable_pose()->mutable_position()->set_z(pose_.Pos().Z());
      // msg.mutable_pose()->mutable_pose()->mutable_orientation()->set_x(pose_.Rot().X());
      // msg.mutable_pose()->mutable_pose()->mutable_orientation()->set_y(pose_.Rot().Y());
      // msg.mutable_pose()->mutable_pose()->mutable_orientation()->set_z(pose_.Rot().Z());

      msg.mutable_twist()->mutable_twist()->mutable_linear()->set_x(linear_vel_.X());
      msg.mutable_twist()->mutable_twist()->mutable_linear()->set_y(linear_vel_.Y());
      msg.mutable_twist()->mutable_twist()->mutable_linear()->set_z(linear_vel_.Z());
      msg.mutable_twist()->mutable_twist()->mutable_angular()->set_x(angular_vel_.X());
      msg.mutable_twist()->mutable_twist()->mutable_angular()->set_y(angular_vel_.Y());
      msg.mutable_twist()->mutable_twist()->mutable_angular()->set_z(angular_vel_.Z());

      this->odom_pub_->publish(msg);
      gzmsg << "[OdomPublisherPlugin] Published odometry data." << std::endl;


    }
  private:
    physics::ModelPtr model_;
    physics::WorldPtr world_;
    physics::LinkPtr link_;

    event::ConnectionPtr update_connection_;
    common::Time last_publish_time_;

    std::shared_ptr<Hnu::Middleware::Node> node_;
    std::shared_ptr<Hnu::Middleware::Publisher<Nav::Odometry>> odom_pub_;

    ignition::math::Vector3d linear_vel_;
    ignition::math::Vector3d angular_vel_;
    ignition::math::Pose3d pose_;



  };
  GZ_REGISTER_MODEL_PLUGIN(OdomPublisherPlugin)
}
