#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <Geometry/Twist.pb.h>

class PubNode: public Hnu::Middleware::Node {
  public:
    PubNode(const std::string &name):Node(name) {
      // Create a publisher
      publisher = createPublisher<Geometry::Twist>("gazebo_d_drive_car");
      // Create a timer
      timer = createTimer(100, [this] { onTimer(); });
    }
    std::shared_ptr<Hnu::Middleware::Publisher<Geometry::Twist>> publisher;
    std::shared_ptr<Hnu::Middleware::Timer> timer;
    double i=1;
    void onTimer() {
      i = i + 0.1;
      Geometry::Twist twist;
      twist.mutable_linear()->set_x(1);
      twist.mutable_angular()->set_z(1);
      publisher->publish(twist);
    }
  };
  


int main()
{
  auto node = std::make_shared<PubNode>("pub_node");
  node->run();
  return 0;
}