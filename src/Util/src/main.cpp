#include"hmw/Node.hpp"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iostream>
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
#include <jsoncpp/json/json.h>
#include<boost/asio/local/stream_protocol.hpp>
#include <boost/program_options.hpp>
namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;
namespace po = boost::program_options;


void json_list(asio::local::stream_protocol::socket& socket, const std::string& key){
    //读取响应
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    std::string response_body = beast::buffers_to_string(res.body().data());
    std::cout << "HTTP Resoponse : \n" << response_body << std::endl;
    //解析json响应
    Json::Reader reader;
    Json::Value jsonData;
    std::string errs;

    if (!reader.parse(response_body, jsonData)){
        std::cerr << "JSON 解析失败： " << reader.getFormattedErrorMessages() << std::endl;
        return ;
    }

    // std::cout << "解析后的JSON数据： " << std::endl;
    // std::cout << jsonData.toStyledString() << std::endl;

    if(jsonData.isMember(key) && jsonData[key].isArray()) {
        std::cout << key << " :" <<std::endl;
        for (const auto& item : jsonData[key]){
            std::cout << item.asString() << std::endl;
        }
    }
    else {
        std::cout << "读取错误！" << std::endl;
    }
}

void send_http_request(const std::string& host, const std::string& target, const std::string& key){
    try{
        asio::io_context io_context;
        asio::local::stream_protocol::socket socket(io_context);
        socket.connect("/tmp/master");
        
        //构造并发送http请求
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        http::write(socket, req);

        //读取响应
        json_list(socket, key);

        socket.shutdown(asio::ip::tcp::socket::shutdown_both);  // 关闭 socket 连接
        socket.close();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


void data_list(const std::string& host, const std::string& target, const std::string& key) {
    std::cout << "Listing all " << key << "..." << std::endl;
    // 实现列出所有话题或节点
    send_http_request(host, target, key);
}

void showTopicInfo(const std::string& topicName) {
    std::cout << "Displaying information for topic: " << topicName << std::endl;
    //显示话题信息
}

void publishTopic(const std::string& topicName, const std::string& message_type, const std::string& message_value) {
    //std::cout << "Publishing message to topic '" << topicName << "': (%s)" << message_type << message_value << std::endl;
    printf("Publishing message to topic '%s': (%s) %s \n", topicName.c_str(), message_type.c_str(), message_value.c_str());
    //发布消息到话题
}

int main(int argc, char* argv[]) {
    try {
        std::string host = "localhost";
        std::string command;
        std::string subcommand;
        std::string TopicName;
        std::string message_type, message_value;

        std::string topic_target = "/topic/list";
        std::string topic_key = "topic";
        std::string node_target = "/node";
        std::string node_key = "node";

        // 定义所有可能的选项
        po::options_description global("Global options");
        global.add_options()
            ("command", po::value<std::string>(&command), "Main command, choose topic or node")
            ("operation", po::value<std::string>(&subcommand), "Subcommand, choose operation")
            ("topic_name", po::value<std::string>(&TopicName), "Topic name to show info")
            ("message_type", po::value<std::string>(&message_type), "Message type")
            ("message_value", po::value<std::string>(&message_value), "Message to publish")
            ("help,h", "Show help message");

        // 定义位置参数解析规则
        po::positional_options_description pos;
        pos.add("command", 1).add("operation", 1).add("topic", 1).add("message_type", 1).add("message_value", 1);

        // 解析命令行
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(global).positional(pos).run(), vm);
        po::notify(vm);
        //std::cout << "size of vm : " << vm.size() << std::endl;

        // std::cout << "Command: " << command << std::endl;
        // std::cout << "Subcommand: " << subcommand << std::endl;

        if (vm.count("help") || command.empty()) {
            std::cout << "Usage: " << argv[0] << " --command [topic|node] --operation [list|info|pub] [options]\n";
            std::cout << global << std::endl;
            return 0;
        }

        //处理命令
        if (command == "topic") {
            if (subcommand == "list") {
                if (vm.size() == 2){
                    data_list(host, topic_target, topic_key);
                }
                else {
                    std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
                }
            } else if (subcommand == "info" ) {
                if (vm.size() == 3){
                    std::cout << "Parsed topic: " << TopicName << std::endl;
                    if (!TopicName.empty()) {
                        showTopicInfo(TopicName);
                    } else {
                        std::cerr << "Error: topic name is required for info" << std::endl;
                    }
                }
                else if (vm.size() == 2){
                    std::cerr << "Error: the following argument is required: topic_name" << std::endl;
                }
                else {
                    std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
                }
            } else if (subcommand == "pub") {
                if (vm.size() == 5){
                    if (!TopicName.empty() && !message_type.empty() && !message_value.empty()) {
                        publishTopic(TopicName, message_type, message_value);
                    } else {
                        std::cerr << "Error: topic name, message type, and message value are required for pub" << std::endl;
                    }
                }
                else if(vm.size() < 5){
                    std::cerr << "Error: topic name, message type, and message value are required for pub" << std::endl;
                }
                else {
                    std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
                }
            } else {
                std::cerr << "Unknown topic subcommand: " << subcommand << std::endl;
            }
        } else if (command == "node") {
            if (subcommand == "list") {
                if (vm.size() == 2){
                    data_list(host, node_target, node_key);
                }
                else {
                    std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
                }
            } else {
                std::cerr << "Unknown node subcommand: " << subcommand << std::endl;
            }
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    } catch (const po::error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}