//
// Created by aoao on 25-3-10.
//

#include <spdlog/spdlog.h>
#include <hmw/Node.hpp>
#include <hmw/Subscriber.hpp>
#include <Std/dgps.pb.h>

int main(){
  spdlog::set_level(spdlog::level::debug);
  auto node=std::make_shared<Hnu::Middleware::Node>("sub_node");
  auto sub_gpfpd = node->createSubscriber<Std::Gpfpd>("_dgps_gpfpd",[](std::shared_ptr<Std::Gpfpd> dgps)
  {
    spdlog::info("str: {}", dgps->status());
  });
  auto sub_gtimu = node->createSubscriber<Std::Gtimu>("_dgps_gtimu",[](std::shared_ptr<Std::Gtimu> dgps)
  {
    spdlog::info("str: {}", dgps->tpr());
  });
  auto sub_gnvtg = node->createSubscriber<Std::Gnvtg>("_dgps_gpfpd",[](std::shared_ptr<Std::Gnvtg> dgps)
  {
    spdlog::info("str: {}", dgps->status());
  });

}
