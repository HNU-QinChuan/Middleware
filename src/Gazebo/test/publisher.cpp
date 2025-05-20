#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <Geometry/PoseStamped.pb.h>

class MyPublisher
{
  public: 
    MyPublisher() 
    {
      this->node = std::make_shared<Hnu::Middleware::Node>("publisher_node");
      this->publisher = node->createPublisher<Geometry::PoseStamped>("publisher_test");
    }
      
    
      std::shared_ptr<Hnu::Middleware::Node> node;
      std::shared_ptr<Hnu::Middleware::Publisher<Geometry::PoseStamped>> publisher;

};
int main()
{
  Geometry::PoseStamped pose;
  pose.mutable_header()->set_frame_id("world");
  pose.mutable_pose()->mutable_position()->set_x(1.0);
  pose.mutable_pose()->mutable_position()->set_y(2.0);
  pose.mutable_pose()->mutable_position()->set_z(3.0);
  MyPublisher myPublisher;
  myPublisher.publisher->publish(pose);
}