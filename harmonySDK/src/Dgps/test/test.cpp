//
// Created by aoao on 25-3-10.
//

#include <spdlog/spdlog.h>
#include <hmw/Node.hpp>
#include <hmw/Subscriber.hpp>
#include <Dgps/Gnvtg.pb.h>
#include <Dgps/Gpfpd.pb.h>
#include <Dgps/Gtimu.pb.h>
#include <Dgps/NavSatFix.pb.h>

int main(){
  spdlog::set_level(spdlog::level::debug);
  auto node=std::make_shared<Hnu::Middleware::Node>("sub_node");
  auto sub_gpfpd = node->createSubscriber<Dgps::Gpfpd>("_dgps_gpfpd",[](std::shared_ptr<Dgps::Gpfpd> dgps)
  {
    spdlog::info("str: {}", dgps->status());
  });
  auto sub_gtimu = node->createSubscriber<Dgps::Gtimu>("_dgps_gtimu",[](std::shared_ptr<Dgps::Gtimu> dgps)
  {
    spdlog::info("str: {}", dgps->tpr());
  });
  auto sub_gnvtg = node->createSubscriber<Dgps::Gnvtg>("_dgps_gnvtg",[](std::shared_ptr<Dgps::Gnvtg> dgps)
  {
    spdlog::info("str: {}", dgps->status());
  });
  auto pub = node->createSubscriber<Dgps::NavSatFix>("dgps", [](std::shared_ptr<Dgps::NavSatFix> dgps)
  {
    spdlog::info("str: {}", dgps->status().status());
  });

  node->run();

}