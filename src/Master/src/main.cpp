//
// Created by yc on 25-2-14.
//
#include "shm/Acceptor.hpp"
#include<spdlog/spdlog.h>
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"

int main(int argc,char* argv[]) {
  spdlog::set_level(spdlog::level::debug);
  Hnu::Interface::InterfaceManager::interfaceManager.init(argv[1]);
  Hnu::Interface::InterfaceManager::interfaceManager.run();
  Hnu::Middleware::MiddlewareManager::run();
}