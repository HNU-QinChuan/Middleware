//
// Created on 2025/5/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "napi_master.h"
#include <iostream>
#include<spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"


//#include "hilog/log.h"
// #undef LOG_DOMAIN
// #undef LOG_TAG
// #define LOG_DOMAIN 0x3200
// #define LOG_TAG "MasterService"

void MasterService(const std::string& hostName){
    spdlog::set_level(spdlog::level::debug);
    //设置 spdlog 输出终端（彩色）
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%^%l%$] %v");

    auto logger = std::make_shared<spdlog::logger>("console", console_sink);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug); // 设置默认等级
    spdlog::info("== MasterService started ==");

    Hnu::Interface::InterfaceManager::interfaceManager.init(hostName);
    Hnu::Interface::InterfaceManager::interfaceManager.run();
    Hnu::Middleware::MiddlewareManager::run();
//     std::thread([]() {
//         try {
//             Hnu::Interface::InterfaceManager::interfaceManager.run();
//             Hnu::Middleware::MiddlewareManager::run();
//         } catch (const std::exception &e) {
//             OH_LOG_ERROR(LOG_APP, "MasterService thread crash: %{public}s", e.what());
//         }
//     }).detach();
}