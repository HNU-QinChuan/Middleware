#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <jsoncpp/json/json.h>
#include <boost/asio/local/stream_protocol.hpp>
#include <spdlog/spdlog.h>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <Std/String.pb.h>
#include <Std/dgps.pb.h>
#include <memory>
#include <google/protobuf/message.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;

//分离出Std和数据类型
std::string transformMessageType(const std::string& message_type) {
    size_t pos = message_type.find('.');
    if (pos != std::string::npos) {
        return message_type.substr(0, pos) + "::" + message_type.substr(pos + 1);
    }
    return message_type;
}

template <typename T>
T json_list(asio::local::stream_protocol::socket& socket, const std::string& key){
    //读取响应
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    std::string response_body = beast::buffers_to_string(res.body().data());
    // std::cout << "HTTP Resoponse : \n" << response_body << std::endl;
    // std::vector<std::string> json_list(const std::string& key){
    T result;
    //解析json响应
    Json::Reader reader;
    Json::Value jsonData;
    std::string errs;

    std::string jsonString = R"({
    "message_type": [
        "int"
    ]
    })";

    if (!reader.parse(response_body, jsonData)){
        std::cerr << "JSON 解析失败： " << reader.getFormattedErrorMessages() << std::endl;
        return result;
    }

    if (!jsonData.isMember(key)) {
        std::cerr << "读取错误: " << key << " 不存在！" << std::endl;
        return result;
    }

    //在编译期间检查数据类型
    if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        // 处理数组
        if (jsonData[key].isArray()) {
            for (const auto& item : jsonData[key]) {
                result.push_back(item.asString());
            }
        } 
    } 
    else if constexpr (std::is_same_v<T, std::string>) {
        // 处理单个字符串
        result = jsonData[key].asString();
    } 
    else {
        std::cout << "读取错误！" << std::endl;
    }
    return result;
}

void send_list_http(const std::string& host, const std::string& target, const std::string& key){
    try{
        asio::io_context io_context;
        asio::local::stream_protocol::socket socket(io_context);
        socket.connect("/tmp/master");
        
        //构造并发送http请求
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        http::write(socket, req);

        //读取响应
        std::vector<std::string> response = json_list<std::vector<std::string>>(socket, key);
        std::cout << key << " :" <<std::endl;
        for (const auto& item : response){
            std::cout << item << std::endl;
        }

        socket.shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);  // 关闭 socket 连接
        socket.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    //json_list(key);
}

std::string send_echo_http(const std::string& host, const std::string& target, const std::string& key, const std::string& topic_name){
    std::string type;
    try{
        asio::io_context io_context;
        asio::local::stream_protocol::socket socket(io_context);
        socket.connect("/tmp/master");
        
        //构造并发送http请求
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set("topic",topic_name);

        http::write(socket, req);

        //读取响应
        type = json_list<std::string>(socket, key);

        socket.shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);  // 关闭 socket 连接
        socket.close();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return type;
}


std::unordered_map<std::string, std::function<void(std::shared_ptr<Hnu::Middleware::Node> node, const std::string& topic_name)>> subscribe_handlers = {
    {"Std::String", [](std::shared_ptr<Hnu::Middleware::Node> node, const std::string& topic_name){
        auto subscriber = node -> createSubscriber<Std::String>(topic_name,
            [](std::shared_ptr<Std::String> message){
                std::cout << "Received data: (" << message -> GetTypeName() << ")" << message -> data() << std::endl;
        });
    }}
    // {"Std::dgps", [](std::shared_ptr<Hnu::Middleware::Node> node, const std::string& topic_name){
    //     auto subscriber = node -> createSubscriber<Std::dgps>(topic_name,
    //         [](std::shared_ptr<Std::dgps> message){
    //             std::cout << "Received data: (" << message -> GetTypeName() << ")" << message -> data() << std::endl;
    //     });
    // }}
};

void subscribe_and_echo(std::shared_ptr<Hnu::Middleware::Node> node, 
                        const std::string& topic_name, 
                        const std::string& message_type){
    std::string std_type = transformMessageType(message_type);
    //使用哈希表存储不同数据类型及对应的function
    if (subscribe_handlers.count(std_type)) {
        subscribe_handlers[std_type](node, topic_name);  // 调用对应的 lambda 处理函数
        std::cout << "Successfully subscribe " << topic_name << ", waiting for message..." << std::endl;
        node -> run();
    } else {
        std::cerr << "Unknown message type: " << std_type << std::endl;
    }

    // if(message_type == "Std.String"){
    //     auto subscriber = node -> createSubscriber<Std::String>(topic_name,
    //         [](std::shared_ptr<Std::String> message){
    //             std::cout << "Received data: (" << message -> GetTypeName() << ")" << message -> data() << std::endl;
    //     });
    // } 
    // else if(message_type == "Std.dgps"){
    //     auto subscriber = node -> createSubscriber<Std::dgps>(topic_name,
    //         [](std::shared_ptr<Std::dgps> message){
    //             std::cout << "Received data: (" << message -> GetTypeName() << ")" << message -> data() << std::endl;
    //     });
    // } 
    // else {
    //     std::cerr << "Unknown Protobuf message type: " << message_type << std::endl;
    // }
    //node -> run();
}

template <typename T>
void publish_message(std::shared_ptr<Hnu::Middleware::Node> node, 
                     const std::string& topic_name, 
                     const T& message) {
    auto publisher = node->createPublisher<T>(topic_name);
    publisher->publish(message);
    std::cout << "Publishing(" << message.GetTypeName() 
              << ") message to topic " << topic_name 
              << ": " << message.DebugString() << std::endl;
}

// 处理函数映射
std::unordered_map<std::string, std::function<void(std::shared_ptr<Hnu::Middleware::Node> node, 
                                const std::string& topic_name, 
                                std::shared_ptr<google::protobuf::Message>& message,
                                int rate, bool rate_flag)>> publish_handlers = {
    {"Std::String", [](std::shared_ptr<Hnu::Middleware::Node> node, const std::string& topic_name,
                            std::shared_ptr<google::protobuf::Message> message,
                            int rate, bool rate_flag) 
        {
            auto msg = std::static_pointer_cast<Std::String>(message);
            if (rate_flag == 0){
                publish_message(node, topic_name, *msg);
            }
            else{
                auto timer = node->createTimer(rate, [node, topic_name, msg]() {
                    publish_message(node, topic_name, *msg);
                });
            }
        }
    }
};

void create_protobuf_message(std::shared_ptr<Hnu::Middleware::Node> node,
    const std::string& topic_name,
    const std::string& message_type, 
    const std::string& message_value,
    int rate, bool rate_flag) 
{
    const google::protobuf::Descriptor* descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(message_type);

    if (descriptor == nullptr) {
        std::cerr << "[ERROR] Cannot found " << message_type
          << " in DescriptorPool" << std::endl;
        return;
    }

    const google::protobuf::Message* prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);

    if (!prototype) {
        std::cerr << "Cannot create Protobuf message: " << message_type << std::endl;
        return;
    }

    std::shared_ptr<google::protobuf::Message> message(prototype->New());

    const google::protobuf::Reflection* reflection = message->GetReflection();
    const google::protobuf::FieldDescriptor* field = descriptor->FindFieldByName("data");

    if (!field) {
        std::cerr << "Protobuf type(" << message_type << ") is missing the 'data' field or has an incorrect type" << std::endl;
        return;
    }

    reflection->SetString(message.get(), field, message_value);

    std::string std_type = transformMessageType(message_type);
    //std::cout << "Replaced type: " << std_type << std::endl;

    //使用哈希表存储不同数据类型及对应的function
    if (publish_handlers.count(std_type)) {
        publish_handlers[std_type](node, topic_name, message, rate, rate_flag);  // 调用对应的 lambda 处理函数
    } else {
        std::cerr << "Unknown message type: " << std_type << std::endl;
    }

    // if (message_type == "Std.String") {
    //     publish_message(node, topic_name, *dynamic_cast<Std::String*>(message.get()));
    // } 
    // // else if (message_type == "Std.dgps") {
    // //     publish_message(node, topic_name, *dynamic_cast<Std::dgps*>(message.get()));
    // // } 
    // else {
    //     std::cerr << "Unknown Protobuf message type: " << message_type << std::endl;
    // }

    node -> run();
    return;
}


void data_list(const std::string& host, const std::string& target, const std::string& key) {
    //列出所有话题或节点
    send_list_http(host, target, key);
}

void showTopicInfo(std::shared_ptr<Hnu::Middleware::Node> node, 
                    const std::string& host, const std::string& target, 
                    const std::string& key, const std::string& topic_name) {
    std::cout << "Displaying information for topic: " << topic_name << std::endl;
    //获取话题数据类型
    std::string type = send_echo_http(host, target, key, topic_name);
    //创建订阅者并输出话题数据
    subscribe_and_echo(node, topic_name, type);
}

void publishTopic(std::shared_ptr<Hnu::Middleware::Node> node, 
                    const std::string& topic_name, 
                    const std::string& message_type, 
                    const std::string& message_value, 
                    int rate, bool rate_flag) {
    //std::cout << "Publishing message to topic '" << topicName << "': (%s)" << message_type << message_value << std::endl;
    //创建发布者
    create_protobuf_message(node, topic_name, message_type, message_value, rate, rate_flag);
}


int main(int argc, char* argv[]){
    //强制注册数据类型
    // protobuf_Std_2fString_2eproto::AddDescriptors();
    // protobuf_Std_2fdgps_2eproto::AddDescriptors();

    std::string host = "localhost";
    std::string topic_target = "/topic";
    std::string topic_key = "topics";
    std::string node_target = "/node";
    std::string node_key = "nodes";
    std::string echo_target = "/topic/info";
    std::string echo_key = "type";
    int rate = 100;
    bool rate_flag = 1;

    std::vector<std::string> tokens;
    for(int i=1; i < argc; i++){
        tokens.push_back(argv[i]);
    }

    int size = tokens.size();
    if(tokens[0] == "node"){
        if (tokens[1] == "list") {
            if (size == 2){
                data_list(host, node_target, node_key);
            }
            else {
                std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        } else {
            std::cerr << "Unknown node subcommand: " << argv[2] << std::endl;
        }
    }
    else if(tokens[0] == "topic"){
        if(tokens[1] == "echo"){
            if (size == 3){
                if (!tokens[2].empty()) {
                    auto echo_node = std::make_shared<Hnu::Middleware::Node>("echo_node");
                    showTopicInfo(echo_node, host, echo_target, echo_key,tokens[2]);
                } else {
                    std::cerr << "Error: topic name is required for echo" << std::endl;
                }
            }
            else if (size == 2){
                std::cerr << "Error: the following argument is required: topic_name" << std::endl;
            }
            else {
                std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        } else if (tokens[1] == "pub") {
            if (size >= 5){
                if (!tokens[2].empty() && !tokens[3].empty() && !tokens[4].empty()) {
                    auto pub_node = std::make_shared<Hnu::Middleware::Node>("publish_node");
                    //确定发布数据的频率
                    if(size == 6){
                        if(tokens[5] == "--once"){
                            rate_flag = 0;
                        }
                        else if(tokens[5] == "--rate"){
                            std::cerr << "Error: rate is required for pub" << std::endl;
                        }
                    }
                    else if(size == 7){
                        if(tokens[5] == "--rate"){
                            rate = std::stoi(tokens[6]);
                        }
                    }
                    publishTopic(pub_node, tokens[2], tokens[3], tokens[4], rate, rate_flag);
                } else {
                    std::cerr << "Error: topic name, message type, and message value are required for pub" << std::endl;
                }
            }
            else if(size < 5){
                std::cerr << "Error: topic name, message type, and message value are required for pub" << std::endl;
            }
            else {
                std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        }
        else if (tokens[1] == "list") {
            if (size == 2){
                data_list(host, topic_target, topic_key);
            }
            else {
                std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
            }
        }
        else {
            std::cerr << "Unknown topic subcommand: " << tokens[1] << std::endl;
        }
    }
    else if(tokens[0] == "-h" || tokens[0] == "--help"){
        std::cout << "Usage: ./client_cmd <topic|node> <list|echo|pub>." << std::endl;
        std::cout << std::endl;
        std::cout << "optional arguments: " << std::endl;
        std::cout << "  -h, --help: show this help message and exit" << std::endl;
        std::cout << std::endl;
        std::cout << "Commands: " << std::endl;
        std::cout << "  topic|node list: show all topics or nodes" << std::endl;
        std::cout << "  topic echo <topic_name>: Display information for <topic_name>" << std::endl;
        std::cout << "  topic pub <topic_name> <message_type> <message_value> (--rate|--once): Publish <message_value> to <topic_name> with <message_type> and unnecessary option (--rate with an integer)" << std::endl;
    }
    else{
        std::cerr << "Unknown command, press [-h] for more detailed usage" <<std::endl;
    }


    return 0;
}
