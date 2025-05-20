#include <iostream>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <Geometry/PoseStamped.pb.h>
#include <Nav/Odometry.pb.h>
#include <TaskPlanner/GlobalPathPlan.pb.h>
#include <Nav/OccupancyGrid.pb.h>

int main()
{
  auto node = std::make_shared<Hnu::Middleware::Node>("subscriber_node");
  auto subscriber = node->createSubscriber<Nav::OccupancyGrid>("gridMap", [](std::shared_ptr<Nav::OccupancyGrid> msg) {
    // std::cout << "Received message: " << msg->DebugString() << std::endl;
    std::cout << "Received message: " << msg->info().width() << " " << msg->info().height() << std::endl;
  });
  node->run();
}