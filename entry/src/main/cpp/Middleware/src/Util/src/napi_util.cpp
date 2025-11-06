//
// Created on 2025/5/7.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "napi_util.h"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/json.h>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <spdlog/spdlog.h>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <thread>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;
namespace fs = std::filesystem;
#include <mutex>
#include "hilog/log.h"
#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0x3200
#define LOG_TAG "util"

// 声明NAPI层导出的函数（跨文件调用）
extern "C" __attribute__((visibility("default"))) void SendToArkTS(int index, const std::string &message);

//保存node列表
static std::unordered_map<std::string, std::shared_ptr<Hnu::Middleware::Node>> g_nodes;
static std::unordered_map<std::string, std::thread> g_threads;
static std::mutex g_mu;

template <typename T>
T json_list(asio::ip::tcp::socket& socket, const std::string& key, std::ostringstream* log)
{
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;

    try {
        http::read(socket, buffer, res);
    } catch (const std::exception& e) {
        if (log) *log << "[HTTP read failed] " << e.what() << "\n";
        return T{};
    }

    std::string response_body = beast::buffers_to_string(res.body().data());

    T result;
    Json::Reader reader;
    Json::Value jsonData;

    if (response_body.empty()) {
        //if (log) *log << "[Error] Response body is empty.\n";
        return result;
    }

    if (!reader.parse(response_body, jsonData)) {
        //if (log) *log << "[Error] Failed to parse JSON: " << reader.getFormattedErrorMessages() << "\n";
        return result;
    }

    if constexpr (std::is_same_v<T, std::map<std::string, std::vector<std::string>>>) {
        for (const auto& member : jsonData.getMemberNames()) {
            const Json::Value& array = jsonData[member];
            std::vector<std::string> items;

            if (array.isArray()) {
                for (const auto& item : array) {
                    items.push_back(item.asString());
                }
            }
            result[member] = items;
        }
    } else if constexpr (std::is_same_v<T, std::string>) {
        result = jsonData[key].asString();
    } else {
        if (log) *log << "[Error] Unsupported return type.\n";
    }

    return result;
}

std::string send_list_http(const std::string& host, const std::string& target, const std::string& key) {
    std::ostringstream log_stream;
    try {
        asio::io_context io_context;
        // 解析 TCP 地址
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");  // 本地端口与 MasterService 一致
        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        http::write(socket, req);

        // 读取响应
        auto response = json_list<std::map<std::string, std::vector<std::string>>>(
            socket, key, &log_stream
        );

        for (const auto& [host_name, items] : response) {
            log_stream << host_name << ": ";
            if (!items.empty()) {
                for (size_t i = 0; i < items.size(); ++i) {
                    log_stream << items[i];
                    if (i < items.size() - 1) log_stream << ", ";
                }
                log_stream << "\n";
            } else {
                log_stream << "(empty)\n";
            }
        }
        
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket.close();
    }
    catch (const boost::system::system_error& e) {
        log_stream << "[FATAL] Boost system error: " << e.what() << "\n";
        log_stream << "[DETAIL] Error code: " << e.code() << ", message: " << e.code().message() << "\n";
        log_stream << "[ERRNO] " << strerror(errno) << " (errno=" << errno << ")\n";
    }
    catch (const std::exception& e) {
        log_stream << "[FATAL] std::exception: " << e.what() << "\n";
    }

    return log_stream.str();
}

//发送topic echo请求，返回数据类型
std::string send_echo_http(const std::string& host, const std::string& target, const std::string& key, const std::string& topic_name, std::ostringstream* log){
    std::string type;
    try{
        asio::io_context io_context;
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");  // 本地端口与 MasterService 一致
        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);
        //构造并发送http请求
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set("topic",topic_name);
        
        http::write(socket, req);

        //读取响应
        type = json_list<std::string>(socket, key, log);

        socket.shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);  // 关闭 socket 连接
        socket.close();

    }
    catch (const std::exception& e) {
        *log << "Error: " << e.what() << std::endl;
    }

    return type;
}

//创建订阅者接收消息
void subscribe_and_echo(const std::string& topic_name, 
                        const std::string& message_type,
                        int index){
    const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(message_type);
    if (descriptor == nullptr) {
        //*log << "[ERROR] Cannot found " << message_type<< " in DescriptorPool" << std::endl;
        std::string logStr = "[ERROR] Cannot found " + message_type + " in DescriptorPool\n";
        OH_LOG_ERROR(LOG_APP, "[SubscribeMessage] %{public}s", logStr.c_str());
        return;
    }
    std::string uniqueName = "echo_node_" + topic_name ;
    auto echo_node = std::make_shared<Hnu::Middleware::Node>(uniqueName);
    auto subscriber = echo_node->createSubscriber<google::protobuf::Message>(
        topic_name, message_type, [echo_node, message_type, index](std::shared_ptr<google::protobuf::Message> message) {
            std::string message_value=message->DebugString();
            if (!message_value.empty() && message_value.back() == '\n') {
                message_value.pop_back();
            }
            std::string logStr = "Received data (" + message_type + "): " + message_value;
            SendToArkTS(index, logStr);
            OH_LOG_DEBUG(LOG_APP,"[SubscribeMessage] %{public}s", logStr.c_str());
        });
    {
        std::lock_guard<std::mutex> lk(g_mu);
        g_nodes[uniqueName] = echo_node;
        std::thread t([echo_node]() { echo_node->run(); });
        g_threads.emplace(uniqueName, std::move(t));
    }
}

void subscribe_and_record(const std::string &topic_name, 
                          const std::string &fileDir,
                          const std::string &message_type,
                          int index) 
{
    const google::protobuf::Descriptor *descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(message_type);
    if (descriptor == nullptr) {
        //*log << "[ERROR] Cannot found " << message_type << " in DescriptorPool" << std::endl;
        std::string logStr="[ERROR] Cannot found "+message_type+" in DescriptorPool\n";
        OH_LOG_ERROR(LOG_APP,"[SubscribeMessage] %{public}s", logStr.c_str());
        return;
    }
    
    ////rosbag record功能，根据话题名-时间创建输出文件
    // 获取当前时间转化为年月日时分秒的格式
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d-%H-%M-%S");
    std::string timestamp = oss.str();
    // 创建对应的文件夹及当前时间点的文件
    fs::path record_dir = fileDir;
    fs::path topic_dir = record_dir / topic_name;
    if (!fs::exists(topic_dir)) {
        fs::create_directories(topic_dir);
    }
    std::string log_file_dir = topic_dir.string() + "_" + timestamp + ".txt";
    OH_LOG_DEBUG(LOG_APP,"[bagrecord]record filename is %{public}s",log_file_dir.c_str());
    std::ofstream log_file(log_file_dir); // 在第一行记录当前话题发布的数据类型
    if (log_file.is_open()) {
        log_file << "message_type: " << message_type << "\n";
        log_file.close();
    } else {
        std::cerr << "Failed to open log file: " << log_file_dir << std::endl;
    }
    
    //接收消息写入文件中
    std::string uniqueName = "record_node_" + topic_name;
    auto record_node = std::make_shared<Hnu::Middleware::Node>(uniqueName);
    // lambda 捕获 shared_ptr<Node> 延长生命周期
    auto subscriber = record_node->createSubscriber<google::protobuf::Message>(
        topic_name, message_type, 
        [record_node, log_file_dir, message_type, index](std::shared_ptr<google::protobuf::Message> message) {
            std::string message_value = message->DebugString();
            if (!message_value.empty() && message_value.back() == '\n') {
                message_value.pop_back();
            }
            std::string logStr = "Record data (" + message_type + "): " + message_value;
            OH_LOG_DEBUG(LOG_APP, "[bagrecord] %{public}s", logStr.c_str());
            SendToArkTS(index,logStr);
            auto now = std::chrono::steady_clock::now();
            static std::unordered_map<std::string, std::chrono::steady_clock::time_point> start_times;
            auto &start_time = start_times[log_file_dir];
            if (start_time.time_since_epoch().count() == 0) start_time = now;

            double elapsed_sec = std::chrono::duration<double>(now - start_time).count();
            std::string message_value_json;
            size_t pos = message_value.find("data:");
            if (pos != std::string::npos) {
                std::string value = message_value.substr(pos + 6);
                if (!value.empty() && value.back() == '\n') value.pop_back();
                message_value_json = "{\"" + message_value.substr(pos, 4) + "\":" + value + "}";
            }

            std::ofstream log_file(log_file_dir, std::ios::app);
            if (log_file.is_open()) {
                log_file << elapsed_sec << "|" << message_value_json << "\n";
                log_file.close();
            } else {
                OH_LOG_ERROR(LOG_APP, "Failed to open log file: %{public}s", log_file_dir.c_str());
            }
        });

    // 加锁保存 Node 和线程
    {
        std::lock_guard<std::mutex> lk(g_mu);
        g_nodes[uniqueName] = record_node;
        std::thread t([record_node]() { record_node->run(); });
        g_threads.emplace(uniqueName, std::move(t));
    }
}
//创建发布者发布消息
template <typename T>
void publish_message(std::shared_ptr<Hnu::Middleware::Node> node,
                     const std::string& topic_name, 
                     const T& message,
                     const std::string& message_type,
                     int index) {
    auto publisher = node->createPublisher<google::protobuf::Message>(topic_name,message_type);
    publisher->publish(message);
    OH_LOG_ERROR(LOG_APP,"[PublishMessage] %{public}s",message.DebugString().c_str());
    //实时打印信息到可视化界面
    std::string debug_info = message.DebugString();
    if (!debug_info.empty() && debug_info.back() == '\n') {
        debug_info.pop_back();
    }
    std::string log_message = "Publish " + debug_info + "(" + message.GetTypeName() + ") to " + topic_name ;
    // 直接调用NAPI层的发送函数（自动处理线程转发）
    SendToArkTS(index, log_message);
}

//protobuf反射数据类型
std::shared_ptr<google::protobuf::Message> create_protobuf_message(/*std::shared_ptr<Hnu::Middleware::Node> node,*/
    //const std::string& topic_name,
    const std::string& message_type,
    const std::string& message_value,
    std::ostringstream* log
    )
{
    const google::protobuf::Descriptor* descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(message_type);

    if (descriptor == nullptr) {
        *log << "[ERROR] Cannot found " << message_type
          << " in DescriptorPool" << std::endl;
        return nullptr;
    }

    const google::protobuf::Message* prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);

    if (!prototype) {
        *log << "Cannot create Protobuf message: " << message_type << std::endl;
        return nullptr;
    }

    std::shared_ptr<google::protobuf::Message> message(prototype->New());

    const google::protobuf::Reflection* reflection = message->GetReflection();

    //解析JSON文件并赋值
    Json::Reader reader;
    Json::Value jsonData;
    if(!reader.parse(message_value,jsonData)){
        *log << "Failed to parse json: " << message_value <<std::endl;
        return nullptr;
    }
    // 打印整个 JSON 数据
    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, jsonData);
    OH_LOG_ERROR(LOG_APP, "[PublishMessage]Parsed JSON content: %{public}s", jsonStr.c_str());
    for (auto it = jsonData.begin(); it != jsonData.end() ; ++it) {
        const google::protobuf::FieldDescriptor* field = descriptor -> FindFieldByName(it.key().asString());
        if(field == nullptr){
           *log << "Protobuf type(" << message_type << ") is missing the "<< it.key().asString() << "field or has an incorrect type" << std::endl;
            return nullptr;
        }
        reflection -> SetString(message.get(), field, it -> asString());
    }

    return message;
}

//创建发布节点，判断发布频率
std::string create_publisher(
                const std::string& topic_name,
                const std::string& message_type,
                std::shared_ptr<google::protobuf::Message>& message,
                int rate, bool rate_flag,
                const std::string& message_value, int index)
{
    std::ostringstream log_stream;
    std::string uniqueName = "pub_node_" + topic_name;
    auto pub_node = std::make_shared<Hnu::Middleware::Node>(uniqueName);
    
    if (rate_flag == 0){
        publish_message(pub_node, topic_name, *message, message_type, index);
    }
    else{
        std::size_t interval_ms = static_cast<std::size_t>(1000.0 / rate);
        auto timer = pub_node->createTimer(interval_ms, [pub_node, topic_name, message, message_type, index]() {
            publish_message(pub_node, topic_name, *message, message_type,  index);
        });
    }
    std::lock_guard<std::mutex> lk(g_mu);
    g_nodes[uniqueName] = pub_node;
    std::thread t([pub_node]() { pub_node->run(); });
    g_threads.emplace(uniqueName, std::move(t));
    return log_stream.str();
}

struct BagMessage{
    double timestamp;
    std::string json_str;
};
void bag_play(const std::string &file_path,
              const std::string &file_name,
              int index) 
{
    auto log_stream = std::make_shared<std::ostringstream>();
    fs::path path = fs::path(file_path) / file_name;
    if (!fs::exists(path)) {
        OH_LOG_DEBUG(LOG_APP, "Bag file does not exist: %{public}s", path.c_str());
        return;
    }
    std::ifstream infile(path);
    if (!infile.is_open()) {
        OH_LOG_DEBUG(LOG_APP,"Failed to open bag file: %{public}s", path.c_str());
        return;
    }

    size_t pos = file_name.find("_");
    std::string topic_name = file_name.substr(0, pos);
    //OH_LOG_DEBUG(LOG_APP,"[bagplay] topic_name is: %{public}s", topic_name.c_str());

    std::string line, message_type;
    std::getline(infile, line);
    if (line.find("message_type: ") == 0) {
        message_type = line.substr(strlen("message_type: "));
    } else {
        std::cerr << "Invalid bag format: no message_type found" << std::endl;
        return;
    }

    std::vector<BagMessage> messages;
    while (std::getline(infile, line)) {
        size_t delim_pos = line.find('|');
        if (delim_pos == std::string::npos) continue;
        BagMessage msg;
        msg.timestamp = std::stod(line.substr(0, delim_pos));
        msg.json_str = line.substr(delim_pos + 1);
        messages.push_back(std::move(msg));
    }
    infile.close();
    if (messages.empty()) return;

    int interval = 0;
    if (messages.size() > 1) {
        interval = static_cast<int>((messages[1].timestamp - messages[0].timestamp) * 1000);
    }

    std::string uniqueName = "play_node_" + topic_name;
    auto play_node = std::make_shared<Hnu::Middleware::Node>(uniqueName);

    auto publisher = play_node->createPublisher<google::protobuf::Message>(topic_name, message_type);
    
    auto state = std::make_shared<size_t>(0);
    if (messages.size() == 1) {
        auto msg = create_protobuf_message(message_type, messages[0].json_str, log_stream.get());
        if (msg) publisher->publish(*msg);
        std::string debug_info = msg->DebugString();
        if (!debug_info.empty() && debug_info.back() == '\n') {
            debug_info.pop_back();
        }
        std::string log_message = "Republish" + debug_info + "(" + msg->GetTypeName() + ") to " + topic_name;
        SendToArkTS(index, log_message);
    } else {
        auto timer = play_node->createTimer(interval, [play_node, publisher, messages, state, message_type, log_stream, index, topic_name]() mutable {
            size_t idx = *state;
            if (idx >= messages.size()) return;
            auto &msginfo = messages[idx];
            auto msg = create_protobuf_message(message_type, msginfo.json_str, log_stream.get());
            if (msg) publisher->publish(*msg);
            std::string debug_info = msg->DebugString();
            if (!debug_info.empty() && debug_info.back() == '\n') {
                debug_info.pop_back();
            }
            std::string log_message = "Republish" + debug_info + "(" + msg->GetTypeName() + ") to " + topic_name;
            SendToArkTS(index, log_message);
            
            (*state)++;
        });
    }

    // 启动 Node，并存储线程到全局 g_threads
    {
        std::lock_guard<std::mutex> lk(g_mu);
        g_nodes[uniqueName] = play_node;
        std::thread t([play_node]() { play_node->run(); });
        g_threads[uniqueName] = std::move(t);
    }
}


std::string data_list(const std::string& host, const std::string& target, const std::string& key) {
    //列出所有话题或节点
    return send_list_http(host, target, key);
}

std::string showTopicEcho(const std::string& host, const std::string& target, 
                    const std::string& key, const std::string& topic_name, int index) {
    //获取话题数据类型
    std::ostringstream log_stream;
    std::string type = send_echo_http(host, target, key, topic_name, &log_stream);
    if(type.empty()){
        log_stream << "Topic (" << topic_name << ") does not appear to be published yet." << std::endl;
        //log_stream << "Type is empty." << "\n";
        return log_stream.str();
    }
    //创建订阅者并输出话题数据
    subscribe_and_echo(topic_name, type, index);
    return log_stream.str();
}

std::string publishTopic(const std::string& topic_name, 
                    const std::string& message_type, 
                    const std::string& message_value, 
                    int rate, bool rate_flag, int index) {
    std::ostringstream log_stream;
    //创建发布者
    auto message = create_protobuf_message(message_type, message_value, &log_stream);
    if(message){
        return create_publisher(topic_name, message_type, message, rate, rate_flag, message_value, index);
    }
    else{
        log_stream << "Failed to create publisher." << "\n";
        return log_stream.str();
    }
}

std::string bag_record(const std::string& host,
                       const std::string& target,
                       const std::string& key,
                       const std::string& topic_name,
                       const std::string& fileDir, int index){
    std::ostringstream log_stream;
    std::string type = send_echo_http(host, target, key, topic_name, &log_stream);
    if (type.empty()) {
        log_stream << "Topic (" << topic_name << ") does not appear to be published yet." << std::endl;
        //OH_LOG_DEBUG(LOG_APP,"[bagrecord] type is empty..");
        //log_stream << "Type is empty."<< "\n";
        return log_stream.str();
    }
    subscribe_and_record(topic_name, fileDir,type, index);
    return log_stream.str();
}

std::string runCommand(const std::vector<std::string>& tokens, const int index) {
    std::ostringstream log_stream ;
    std::string host = "localhost";
    std::string topic_target = "/topic";
    std::string topic_key = "topics";
    std::string node_target = "/node";
    std::string node_key = "nodes";
    std::string echo_target = "/topic/info";
    std::string echo_key = "type";
    int rate = 10;
    bool rate_flag = 1;

    int size = tokens.size();
    if(tokens[0] == "node"){
        if (tokens[1] == "list") {
            if (size == 2){
                return data_list(host, node_target, node_key);
            }
            else {
                log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        } else {
            log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
        }
    }
    else if(tokens[0] == "topic"){
        if(tokens[1] == "echo"){
            if (size == 3){
                if (!tokens[2].empty()) {
                    return showTopicEcho(host, echo_target, echo_key,tokens[2], index);
                } else {
                    log_stream << "Error: topic name is required for echo" << std::endl;
                }
            }
            else if (size == 2){
                log_stream << "Error: the following argument is required: topic_name" << std::endl;
            }
            else {
                log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        } else if (tokens[1] == "pub") {
            if (size >= 5){
                if (!tokens[2].empty() && !tokens[3].empty() && !tokens[4].empty()) {
                    //确定发布数据的频率
                    if(size == 6){
                        if(tokens[5] == "--once"){
                            rate_flag = 0;
                        }
                        else if(tokens[5] == "--rate"){
                            log_stream << "Error: rate is required for pub" << std::endl;
                        }
                    }
                    else if(size == 7){
                        if(tokens[5] == "--rate"){
                            rate = std::stoi(tokens[6]);
                        }
                    }
                    return publishTopic(tokens[2], tokens[3], tokens[4], rate, rate_flag, index);
                } else {
                    log_stream<< "Error: topic name, message type, and message value are required for pub" << std::endl;
                }
            }
            else if(size < 5){
                log_stream << "Error: topic name, message type, and message value are required for pub" << std::endl;
            }
            else {
                log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        }
        else if (tokens[1] == "list") {
            if (size == 2){
                return data_list(host, topic_target, topic_key);
            }
            else {
                log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        }
        else {
            log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
        }
    }
    else if(tokens[0] == "bag") {
        if(tokens[1]=="record"){
            if(size == 4){ //在NAPI侧传递存储位置
            //创建订阅者记录
                std::string path = tokens[3];
                //OH_LOG_DEBUG(LOG_APP,"[bagrecord] path is : %{public}s", path.c_str());
                return bag_record(host, echo_target, echo_key, tokens[2], path, index);
            }
            else{
                log_stream << "Error: topic_name is required for record" << std::endl;
            }
        }
        else if (tokens[1] == "play") {
            if (size == 4){
                ////读取对应的文件
                std::string path=tokens[3]+"/"+tokens[2];
                OH_LOG_DEBUG(LOG_APP, "[bagplay] path is %{public}s", path.c_str());
                bag_play(tokens[3], tokens[2], index);
            }
            else {
                log_stream << "Error: file_path is required for play" << std::endl;
            }
        }
        else {
            log_stream << "Invalid command, press [-h] for more detailed usage" << std::endl;
        }
    }
    else if(tokens[0] == "-h" || tokens[0] == "--help"){
        log_stream << "Usage: <topic|node|bag> <list|echo|pub|play>.\n\n";
        log_stream << "optional arguments:\n";
        log_stream << "  -h, --help: show this help message and exit\n\n";
        log_stream << "Commands:\n";
        log_stream << "  topic|node list: show all topics or nodes\n";
        log_stream << "  topic echo <topic_name>: Display information for <topic_name>\n";
        log_stream << "  topic pub <topic_name> <message_type> <message_value> (--rate|--once):\n";
        log_stream << "    Publish <message_value> to <topic_name> with <message_type> and optional rate settings\n";
        log_stream << "  bag record <topic_name>: \n";
        log_stream << "    Record data of <topic_name> to a file starting from now.\n";
        log_stream << "  bag play <file_path(topic_name/Y-M-D-H-M-S.txt)>: \n";
        log_stream << "    Replay information for <topic_name> at time <Y-M-D-H-M-S>\n";
    }
    else{
        log_stream << "Unknown command, press [-h] for more detailed usage" << std::endl;
    }
    return log_stream.str();
}

void stopCommand(const std::string& op, const std::string& topic) {
    std::string target="/node/stop";
    std::string host = "localhost";
    
    std::string nodeName;
    if(op=="play"){
        size_t pos = topic.find("_");
        std::string topic_name = topic.substr(0, pos);
        nodeName=op + "_node_" + topic_name;
    }
    else nodeName=op + "_node_" + topic;

    std::shared_ptr<Hnu::Middleware::Node> node;
    std::thread t;

    {
        std::lock_guard<std::mutex> lk(g_mu);
        auto itNode = g_nodes.find(nodeName);
        if(itNode != g_nodes.end()) {
            node = itNode->second;
        }
        auto itThread = g_threads.find(nodeName);
        if(itThread != g_threads.end()) {
            t = std::move(itThread->second);
            g_threads.erase(itThread);
        }
        g_nodes.erase(nodeName);
    }

    if(node) {
        OH_LOG_DEBUG(LOG_APP,"[stopCommand]Stopping node %{public}s", nodeName.c_str());
        node->stop();
    }

    if(t.joinable()) {
        OH_LOG_DEBUG(LOG_APP,"[stopCommand]Joining thread %{public}s", nodeName.c_str());
        t.join();
        OH_LOG_DEBUG(LOG_APP,"[stopCommand]Joined thread %{public}s", nodeName.c_str());
    }
    try {
        asio::io_context io_context;
        // 解析 TCP 地址
        asio::ip::tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");  // 本地端口与 MasterService 一致
        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        // 构造 POST 请求
        http::request<http::string_body> req{http::verb::post, "/node/stop", 11};
        req.set(http::field::host, "localhost");
        req.set(http::field::content_type, "application/json");

        // body 里传 node 名
        req.body() = "{\"node\":\"" + nodeName + "\"}";
        OH_LOG_DEBUG(LOG_APP,"[stopCommand]req_body is %{public}s", req.body().c_str());
        req.prepare_payload();

        // 发送请求
        http::write(socket, req);

        // 读取响应
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(socket, buffer, res);
        
        OH_LOG_DEBUG(LOG_APP,"[stopCommand]Response: %{public}d  %{public}s",res.result_int(),res.body().c_str());
        //std::cout << "Response: " << res.result_int() << " " << res.body() << std::endl;
    } 
    catch (const std::exception& e) {
        OH_LOG_DEBUG(LOG_APP,"[stopCommand]Exception: %{public}s", e.what());
        //std::cerr << "Exception: " << e.what() << std::endl;
    }
}
