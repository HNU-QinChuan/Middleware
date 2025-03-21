//
// Created by yc on 25-2-14.
//
#include "shm/Acceptor.hpp"
#include<spdlog/spdlog.h>
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);
  Hnu::Interface::InterfaceManager::interfaceManager.init("host1");
  Hnu::Middleware::MiddlewareManager::run();
}