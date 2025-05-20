#include <gazebo/gazebo.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/common/common.hh>
#include <Geometry/Twist.pb.h>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <thread>

namespace gazebo
{
  class DiffDriveCarPlugin : public ModelPlugin
  {
    public:
      void Load(physics::ModelPtr _model, sdf::ElementPtr _sdf) override
      {
        gzmsg << "[DiffDriveZmqPlugin] 插件已加载成功。" << std::endl;
        this->model = _model;
        this->leftJoint = model->GetJoint("left_wheel_joint");
        this->rightJoint = model->GetJoint("right_wheel_joint");
        if(!this->leftJoint || !this->rightJoint)
        {
          gzerr << "Left or right joint not found!" << std::endl;
          return;
        }
        
        if (_sdf->HasElement("wheelSeparation"))
        {
          this->wheelSeparation = _sdf->Get<double>("wheelSeparation");
          gzmsg << "[DiffDrivePlugin] 读取 wheelSeparation = " << this->wheelSeparation << std::endl;
        }
        else
        {
          gzmsg << "[DiffDrivePlugin] 未设置 wheelSeparation，使用默认值 = 0.4" << std::endl;
        }

        node = std::make_shared<Hnu::Middleware::Node>("drive_car_plugin_node");
        subscriber = node->createSubscriber<Geometry::Twist>("cmd_vel",std::bind(&DiffDriveCarPlugin::CmdCallback, this, std::placeholders::_1));


        running = true;
        nodeThread = std::thread([this]() {
          this->node->run();
        });

        this->updateConnection = event::Events::ConnectWorldUpdateBegin(
          std::bind(&DiffDriveCarPlugin::OnUpdate, this));
      }


      void CmdCallback(std::shared_ptr<Geometry::Twist> cmd)
      {
        std::lock_guard<std::mutex> lock(cmdMutex);
        double linear = cmd->linear().x();
        double angular = cmd->angular().z();
        leftVel = linear - angular * wheelSeparation / 2.0;
        rightVel = linear + angular * wheelSeparation / 2.0;
        gzmsg << "[DiffDrivePlugin] 线速度: " << linear << ", 角速度: " << angular << std::endl;
      }

      void OnUpdate()
      {
        std::lock_guard<std::mutex> lock(cmdMutex);
        leftJoint->SetVelocity(0, leftVel);
        rightJoint->SetVelocity(0, rightVel);
      }

      void Reset() override
      {
        //关闭节点，等待添加
      }

    private:
      physics::ModelPtr model;
      physics::JointPtr leftJoint;
      physics::JointPtr rightJoint;

      event::ConnectionPtr updateConnection;
      std::shared_ptr<Hnu::Middleware::Node> node;
      std::shared_ptr<Hnu::Middleware::Subscriber<Geometry::Twist>> subscriber;
      std::thread nodeThread;
      bool running = false;
      double leftVel = 0.0;
      double rightVel = 0.0;

      std::mutex cmdMutex;

      double wheelSeparation = 0.4; // 轮间距
  };
  GZ_REGISTER_MODEL_PLUGIN(DiffDriveCarPlugin)
}