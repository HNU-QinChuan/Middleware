#include <iostream>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <Geometry/PoseStamped.pb.h>
#include <Nav/Odometry.pb.h>

int main()
{
  auto node = std::make_shared<Hnu::Middleware::Node>("subscriber_node");
  auto subscriber = node->createSubscriber<Nav::Odometry>("gazebo_odom_topic", [](std::shared_ptr<Nav::Odometry> msg) {
    std::cout << "Received message: " << msg->DebugString() << std::endl;
  });
  node->run();
}