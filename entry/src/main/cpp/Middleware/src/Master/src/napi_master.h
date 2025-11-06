//
// Created on 2025/5/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#ifndef MID_NAPI_NAPI_MASTER_H
#define MID_NAPI_NAPI_MASTER_H

#include <string>

void MasterService(const std::string& hostName);

#endif //MID_NAPI_NAPI_MASTER_H