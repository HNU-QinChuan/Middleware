#include "napi/native_api.h"
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iostream>
#include <thread>
#include <atomic>
#include <spdlog/spdlog.h>
#include "src/Util/src/napi_util.h"
#include "hilog/log.h"
#include <uv.h>
#include <queue>
#include <mutex>

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200
#define LOG_TAG "NAPI"

namespace fs = std::filesystem;

// 全局状态
struct {
    napi_env env = nullptr;
    napi_ref callback = nullptr;
    uv_async_t async_handle;
    bool initialized = false;
    std::mutex mutex;
    std::queue<std::string> messages;
} g_state;

extern "C" __attribute__((visibility("default"))) void SendToArkTS(int index, const std::string &message);

// C++ -> ArkTS 消息发送
void SendToArkTS(int index, const std::string &message) {
    std::lock_guard<std::mutex> lock(g_state.mutex);
    if (!g_state.env || !g_state.callback) {
        OH_LOG_ERROR(LOG_APP, "[OnAsyncMessage] env or callback missing");
        return;
    }

    std::ostringstream oss;
    oss << index << "|" << message;
    g_state.messages.push(oss.str());

    // 打印当前线程 ID，确认是否在子线程触发
    OH_LOG_ERROR(LOG_APP, "[OnAsyncMessage] push msg='%{public}s' thread=%zu", oss.str().c_str(),
                 std::hash<std::thread::id>{}(std::this_thread::get_id()));

    // 唤醒 ArkTS 主线程
    uv_async_send(&g_state.async_handle);
}

// 主线程异步回调，由 libuv 触发
static void OnAsyncMessage(uv_async_t *handle) {
    std::lock_guard<std::mutex> lock(g_state.mutex);

    if (!g_state.env || !g_state.callback) {
        OH_LOG_ERROR(LOG_APP, "[OnAsyncMessage] env or callback missing");
        return;
    }

    OH_LOG_ERROR(LOG_APP, "[OnAsyncMessage] messages size=%{public}zu", g_state.messages.size());

    // 逐条处理
    while (!g_state.messages.empty()) {
        std::string msg = std::move(g_state.messages.front());
        g_state.messages.pop();

        napi_value js_msg;
        napi_create_string_utf8(g_state.env, msg.c_str(), msg.size(), &js_msg);

        napi_value js_callback;
        napi_get_reference_value(g_state.env, g_state.callback, &js_callback);

        napi_value result;
        napi_status status = napi_call_function(g_state.env, nullptr, js_callback, 1, &js_msg, &result);

        if (status != napi_ok) {
            OH_LOG_ERROR(LOG_APP, "[OnAsyncMessage] napi_call_function failed: %d", status);
        }
    }
}

// 注册 ArkTS 回调函数
static napi_value RegisterMessageCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_valuetype type;
    napi_typeof(env, args[0], &type);
    if (type != napi_function) {
        napi_throw_type_error(env, nullptr, "Argument must be a function");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.env = env;

    // 删除旧的引用
    if (g_state.callback != nullptr) {
        napi_delete_reference(env, g_state.callback);
    }

    // 保存新回调引用
    napi_create_reference(env, args[0], 1, &g_state.callback);

    // 只初始化一次
    if (!g_state.initialized) {
        uv_loop_t *loop;
        napi_get_uv_event_loop(env, &loop); // 用 napi 提供的 event loop
        uv_async_init(loop, &g_state.async_handle, OnAsyncMessage);
        g_state.initialized = true;
        OH_LOG_ERROR(LOG_APP, "[RegisterMessageCallback] uv_async_init done");
    }

    return nullptr;
}


// //NAPI封装
napi_value RunCommand(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    // 获取输入字符串
    size_t str_size = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &str_size);
    std::string input(str_size + 1, 0);
    napi_get_value_string_utf8(env, args[0], &input[0], str_size + 1, &str_size);
    input.resize(str_size);

    std::vector<std::string> tokens;

    // 匹配被单引号包裹的 JSON 内容
    std::regex jsonRegex(R"###("(\{.*?\})")###"); 
    std::smatch match;

    if (std::regex_search(input, match, jsonRegex)) {
        std::string jsonStr = match[1];  // 获取 JSON 部分
        std::string beforeJson = input.substr(0, match.position());
        std::string afterJson = input.substr(match.position() + match.length());

        // 拆分 JSON 前部分
        std::istringstream iss1(beforeJson);
        std::string token;
        while (iss1 >> token) {
            //OH_LOG_ERROR(LOG_APP, "[PublishMessage] %{public}s", token.c_str());
            tokens.push_back(token);
        }

        tokens.push_back(jsonStr);  // 加入清洗过的 JSON 内容
        //OH_LOG_ERROR(LOG_APP, "[PublishMessage] %{public}s", jsonStr.c_str());
        // 拆分 JSON 后部分
        std::istringstream iss2(afterJson);
        while (iss2 >> token) {
            //OH_LOG_ERROR(LOG_APP, "[PublishMessage] %{public}s", token.c_str());
            tokens.push_back(token);
        }
    } else {
        // 无 JSON，直接按空格分割
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
    }
    // 如果是bag record/bag play再传入路径
    if (tokens.size() >= 2 && tokens[0] == "bag" && (tokens[1] == "record"||tokens[1]=="play")) {
        // 这里获取第二个参数 record_path
        if (argc >= 2) {
            napi_value argPath;
            napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
            size_t path_size = 0;
            napi_get_value_string_utf8(env, args[1], nullptr, 0, &path_size);
            std::string record_path(path_size + 1, 0);
            napi_get_value_string_utf8(env, args[1], &record_path[0], path_size + 1, &path_size);
            record_path.resize(path_size);
            OH_LOG_DEBUG(LOG_APP, "[bagrecord] %{public}s", record_path.c_str());
            tokens.push_back(record_path);
        }
    }
    int32_t index;
    napi_get_value_int32(env, args[2], &index);
    // 调用原始命令函数
    std::string testStr = runCommand(tokens,index);

    napi_value result;
    napi_create_string_utf8(env, testStr.c_str(), testStr.length(), &result);
    //return nullptr;
    return result;
}

napi_value StopRuncommand(napi_env env, napi_callback_info info){
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    // 获取输入字符串
    size_t str_size = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &str_size);
    std::string input(str_size + 1, 0);
    napi_get_value_string_utf8(env, args[0], &input[0], str_size + 1, &str_size);
    input.resize(str_size);

    std::vector<std::string> tokens;
    
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if(tokens[1]!="list"){
        stopCommand(tokens[1],tokens[2]);
        std::string res = "Stop.";
        napi_value result;
        napi_create_string_utf8(env, res.c_str(), res.length(), &result);
    
        return result;
    }
    return nullptr;
}

//模块初始化,实现ArkTS接口与C++接口的绑定和映射。
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"runCommand", nullptr, RunCommand, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stopCommand", nullptr, StopRuncommand, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"registerMessageCallback", nullptr, RegisterMessageCallback, nullptr, nullptr, nullptr, napi_default,nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

// 准备模块加载相关信息，将上述Init函数与本模块名等信息记录下来。
static napi_module testModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

// 加载so时，该函数会自动被调用，将上述testModule模块注册到系统中。
extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&testModule);
}
