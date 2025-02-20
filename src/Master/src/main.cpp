//
// Created by yc on 25-2-14.
//
#include "Acceptor.hpp"
#include<spdlog/spdlog.h>
#include "MiddlewareManager.hpp"

int main() {
  // spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [Hnu Middleware Master] %v");
  spdlog::set_level(spdlog::level::debug);
  spdlog::debug("Hello, Middleware-Master");
  Hnu::Middleware::Acceptor acceptor;
  acceptor.run();
  Hnu::Middleware::MiddlewareManager::run();
}