#include <QtCore>
#include <gazebo/gui/GuiPlugin.hh>
#include <gazebo/rendering/UserCamera.hh>
#include <gazebo/rendering/Scene.hh>
#include <gazebo/rendering/rendering.hh>
#include <gazebo/gui/MouseEventHandler.hh>
#include <gazebo/gui/GuiIface.hh>
#include <gazebo/gui/GuiEvents.hh>
#include <gazebo/common/Events.hh>
#include <gazebo/common/Plugin.hh>
#include <ignition/math/Vector3.hh>

#include <Geometry/PoseStamped.pb.h>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>

namespace gazebo
{
  class GuiMouseClickGoal : public GUIPlugin
  {
    public: 
    GuiMouseClickGoal() : GUIPlugin()
    {
      gzmsg << "[GuiMouseClickGoal] 插件已加载成功。" << std::endl;
      this->node = std::make_shared<Hnu::Middleware::Node>("gui_mouse_click_goal_node");
      this->publisher = node->createPublisher<Geometry::PoseStamped>("gazebo_gui_mouse_click_goal");
      this->resize(50, 50);
      this->setFocusPolicy(Qt::StrongFocus);
      this->setFocus();  // 保证能接收键盘事件
      
      // 添加鼠标事件过滤器
      gui::MouseEventHandler::Instance()->AddPressFilter(
        "mouse_click_goal_gui_click",
        std::bind(&GuiMouseClickGoal::OnLeftClick, this, std::placeholders::_1)
      );
      gui::MouseEventHandler::Instance()->AddReleaseFilter(
        "mouse_click_goal_gui_release",
        std::bind(&GuiMouseClickGoal::OnLeftRelease, this, std::placeholders::_1)
      );
      gui::MouseEventHandler::Instance()->AddMoveFilter(
        "mouse_click_goal_gui_move",
        std::bind(&GuiMouseClickGoal::OnMouseMove, this, std::placeholders::_1)
      );
    }

    ~GuiMouseClickGoal()
    {
      gui::MouseEventHandler::Instance()->RemovePressFilter("mouse_click_goal_gui_click");
      gui::MouseEventHandler::Instance()->RemoveReleaseFilter("mouse_click_goal_gui_release");
      gui::MouseEventHandler::Instance()->RemoveMoveFilter("mouse_click_goal_gui_move");
    }

    private:
      bool OnLeftClick(const common::MouseEvent &event)
      {
        if(!this->goal_enabled)
          return false;

        rendering::UserCameraPtr camera = gui::get_active_camera();
        if (!camera)
          return false;

        ignition::math::Vector3d intersection;
        if (!camera->GetScene()->FirstContact(camera, event.Pos(), intersection))
          return false;

        start_pos = intersection;
        has_start = true;
        is_dragging = true;

        gzmsg << "[GuiMouseClickGoal] 鼠标按下，记录起点: ("
              << start_pos.X() << ", " << start_pos.Y() << ", " << start_pos.Z() << ")" << std::endl;

        return true;  // 返回true阻止事件继续传播，防止相机移动
      }

      bool OnMouseMove(const common::MouseEvent &event)
      {
        // 只有在拖动状态下才处理移动事件
        if (this->goal_enabled && this->is_dragging)
        {
          // 阻止事件传播到相机控制
          return true;
        }
        
        return false;
      }

      bool OnLeftRelease(const common::MouseEvent &event)
      {
        if(!this->goal_enabled || !has_start)
          return false;

        is_dragging = false;
        
        rendering::UserCameraPtr camera = gui::get_active_camera();
        if (!camera)
          return false;

        ignition::math::Vector3d end_pos;
        if (!camera->GetScene()->FirstContact(camera, event.Pos(), end_pos))
          return false;

        if ((end_pos - start_pos).Length() < 1e-3)
        {
          gzmsg << "[GuiMouseClickGoal] 拖动距离太小，忽略此次发布。" << std::endl;
          has_start = false;
          return false;
        }

        // 计算方向向量
        ignition::math::Vector3d dir = end_pos - start_pos;
        double yaw = std::atan2(dir.Y(), dir.X());
        ignition::math::Quaterniond quat(0, 0, yaw);

        Geometry::PoseStamped msg;
        msg.mutable_header()->set_frame_id("map");
        msg.mutable_pose()->mutable_position()->set_x(start_pos.X());
        msg.mutable_pose()->mutable_position()->set_y(start_pos.Y());
        msg.mutable_pose()->mutable_position()->set_z(start_pos.Z());
        msg.mutable_pose()->mutable_orientation()->set_x(quat.X());
        msg.mutable_pose()->mutable_orientation()->set_y(quat.Y());
        msg.mutable_pose()->mutable_orientation()->set_z(quat.Z());
        msg.mutable_pose()->mutable_orientation()->set_w(quat.W());

        this->publisher->publish(msg);

        gzmsg << "[GuiMouseClickGoal] 鼠标释放，发布目标点: pos("
              << start_pos.X() << ", " << start_pos.Y() << ", " << start_pos.Z()
              << ") quat(" << quat.X() << ", " << quat.Y() << ", " << quat.Z() << ", " << quat.W() << ")" << std::endl;

        has_start = false;
        
        return true;  // 返回true阻止事件继续传播
      } 

      void keyPressEvent(QKeyEvent *event) override
      {
        gzmsg << "[GuiMouseClickGoal] keyPressEvent key: " << event->key() << std::endl;
        if (event->key() == Qt::Key_G)
        {
          this->goal_enabled = !this->goal_enabled;
          
          // 切换模式时重置状态
          this->has_start = false;
          this->is_dragging = false;
          
          // 输出当前模式信息
          gzmsg << "[GuiMouseClickGoal] 鼠标点击发送功能已 "
                << (this->goal_enabled ? "启用" : "禁用") << std::endl;
                
          // 如果进入目标模式，可以显示提示信息
        }
      }

    private:
      std::shared_ptr<Hnu::Middleware::Node> node;
      std::shared_ptr<Hnu::Middleware::Publisher<Geometry::PoseStamped>> publisher;
      bool goal_enabled = false;
      ignition::math::Vector3d start_pos;  // 记录第一次点击的位置
      bool has_start = false;              // 标记是否已有起点
      bool is_dragging = false;            // 标记是否正在拖动
  };
  
  GZ_REGISTER_GUI_PLUGIN(GuiMouseClickGoal)
}