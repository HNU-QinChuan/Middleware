//
// Created on 2025/5/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef MID_NAPI_NAPI_UTIL_H
#define MID_NAPI_NAPI_UTIL_H

#include <string>
#include <vector>
#include <functional>

std::string runCommand(const std::vector<std::string>& tokens, const int index);
void stopCommand(const std::string& op, const std::string& topic);


#endif //MID_NAPI_NAPI_UTIL_H
