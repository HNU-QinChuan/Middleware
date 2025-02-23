//
// Created by yc on 25-2-14.
//
#include "Acceptor.hpp"
#include<spdlog/spdlog.h>
#include "MiddlewareManager.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);
  Hnu::Middleware::Acceptor acceptor;
  acceptor.run();
  Hnu::Middleware::MiddlewareManager::run();
}