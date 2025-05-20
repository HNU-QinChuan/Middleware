#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <Nav/OccupancyGrid.pb.h>
#include <thread>

int main()
{
  auto node = std::make_shared<Hnu::Middleware::Node>("map_publisher_node");
  auto map_pub_ = node->createPublisher<Nav::OccupancyGrid>("gridMap");

  auto map = std::make_shared<Nav::OccupancyGrid>();

    // Header
    map->mutable_header()->set_frame_id("odom");

    // Map metadata
    Nav::MapMetaData* info = map->mutable_info();
    info->set_resolution(0.1f);  // 每个栅格 0.1 米
    info->set_width(100);        // 10 米
    info->set_height(100);       // 10 米

    // 时间戳（可选）
    auto map_time = info->mutable_map_load_time();
    map_time->set_sec(0);        // 或设置为当前时间
    map_time->set_nanosec(0);

    // 地图原点
    auto* origin = info->mutable_origin();
    origin->mutable_position()->set_x(0.0);
    origin->mutable_position()->set_y(0.0);
    origin->mutable_position()->set_z(0.0);
    origin->mutable_orientation()->set_w(1.0);  // 单位四元数
    origin->mutable_orientation()->set_x(0.0);
    origin->mutable_orientation()->set_y(0.0);
    origin->mutable_orientation()->set_z(0.0);

    // Data 全为 0（自由空间）
    map->mutable_data()->Reserve(100 * 100);
    for (int i = 0; i < 100 * 100; ++i) {
        map->mutable_data()->Add(0);
    }

    while(true)
    {
      map_pub_->publish(*map);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}