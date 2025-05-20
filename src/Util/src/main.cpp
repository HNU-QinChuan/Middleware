#include <cstddef>
#include <cstring>
#include <iostream>
#include <csignal>
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
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/algorithm/string.hpp>
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

template <typename T>
T json_list(asio::local::stream_protocol::socket& socket, const std::string& key){
    //读取响应
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res);

    std::string response_body = beast::buffers_to_string(res.body().data());
    //std::cout << "HTTP Resoponse : \n" << response_body << std::endl;

    T result;
    //解析json响应
    Json::Reader reader;
    Json::Value jsonData;
    std::string errs;

    if (response_body.empty()){
        return result;
    }

    if (!reader.parse(response_body, jsonData)){
        std::cerr << "Failed to parse json: " << reader.getFormattedErrorMessages() << std::endl;
        return result;
    }

    if constexpr (std::is_same_v<T,std::map<std::string, std::vector<std::string>>>) {
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
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        // 处理单个字符串
        result = jsonData[key].asString();
        // std::cout << result << std::endl;
    } 
    else {
        std::cout << "Failed to read." << std::endl;
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
        std::map<std::string, std::vector<std::string>> response = json_list<std::map<std::string, std::vector<std::string>>>(socket,key);

        // 输出每个 host 的结果
        for (const auto& [host_name, items] : response) {
            std::cout << host_name << ": \n";
            if (!items.empty()) {
                std::cout << "  ";
                for (size_t i = 0; i < items.size(); ++i) {
                    std::cout << items[i];
                    if (i < items.size() - 1) std::cout << ", ";
                }
                std::cout << "\n";
            } else {
                std::cout << " ";
            }
            std::cout << "\n";
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
        if(type.empty()){
            std::cerr << "Topic (" << topic_name << ") does not appear to be published yet." << std::endl;
        }

        socket.shutdown(boost::asio::local::stream_protocol::socket::shutdown_both);  // 关闭 socket 连接
        socket.close();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return type;
}


void subscribe_and_echo(/*std::shared_ptr<Hnu::Middleware::Node> node, */
                        const std::string& topic_name, 
                        const std::string& message_type){
    const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(message_type);
    if (descriptor == nullptr) {
        std::cerr << "[ERROR] Cannot found " << message_type
          << " in DescriptorPool" << std::endl;
        return;
    }
    auto echo_node = std::make_shared<Hnu::Middleware::Node>("echo_node");
    auto subscriber = echo_node->createSubscriber<google::protobuf::Message>(topic_name, message_type, [&](std::shared_ptr<google::protobuf::Message> message){
        std::cout << "Received data (" << message_type << "): " << message -> DebugString() << std::endl;
    });
    echo_node -> run();
}

template <typename T>
void publish_message(std::shared_ptr<Hnu::Middleware::Node> node,
                     const std::string& topic_name, 
                     const T& message,
                     const std::string& message_type,
                     const std::string& log_file_dir,
                     const std::string& message_value) {
    auto publisher = node->createPublisher<google::protobuf::Message>(topic_name,message_type);
    publisher->publish(message);

    ///把发布的消息写入对应的txt中
    auto now = std::chrono::steady_clock::now();
    ///判断此日志是否是第一次被写入
    static std::unordered_map<std::string, std::chrono::steady_clock::time_point> start_times;
    auto& start_time = start_times[log_file_dir];
    if (start_time.time_since_epoch().count() == 0) {
        start_time = now;   //如果是第一次写入则记录当前时间点为初始时间
    }
    double elapsed_sec = std::chrono::duration<double>(now - start_time).count();  //计算每次发布的消息与第一条消息的间隔以便play
    ////消息以JSON格式写入
    std::ofstream log_file(log_file_dir, std::ios::app);
    if (log_file.is_open()) {
        log_file << elapsed_sec << "|" << message_value << "\n";
        log_file.close();
    } else {
        std::cerr << "Failed to open log file: " << log_file_dir << std::endl;
    }
    // 将消息序列化为二进制
    // std::string binary_data;
    // if (!message.SerializeToString(&binary_data)) {
    //     std::cerr << "Failed to serialize protobuf message to string." << std::endl;
    //     return;
    // }

    // // 打开日志文件，写入时间戳和二进制长度 + 内容（防止 | 冲突）
    // std::ofstream log_file(log_file_dir, std::ios::app | std::ios::binary);
    // if (log_file.is_open()) {
    //     uint32_t data_size = binary_data.size();
    //     log_file.write(reinterpret_cast<const char*>(&elapsed_sec), sizeof(elapsed_sec));  // 时间戳
    //     log_file.write(reinterpret_cast<const char*>(&data_size), sizeof(data_size));      // 长度
    //     log_file.write(binary_data.data(), data_size);                                     // 二进制数据
    //     log_file.close();
    // } else {
    //     std::cerr << "Failed to open log file: " << log_file_dir << std::endl;
    // }

    std::cout << "Publishing(" << message.GetTypeName() 
              << ") message to topic " << topic_name 
              << ": " << message.DebugString() << std::endl;
}

std::shared_ptr<google::protobuf::Message> create_protobuf_message(/*std::shared_ptr<Hnu::Middleware::Node> node,*/
    //const std::string& topic_name,
    const std::string& message_type, 
    const std::string& message_value
    ) 
{
    const google::protobuf::Descriptor* descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(message_type);

    if (descriptor == nullptr) {
        std::cerr << "[ERROR] Cannot found " << message_type
          << " in DescriptorPool" << std::endl;
        return nullptr;
    }

    const google::protobuf::Message* prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);

    if (!prototype) {
        std::cerr << "Cannot create Protobuf message: " << message_type << std::endl;
        return nullptr;
    }

    std::shared_ptr<google::protobuf::Message> message(prototype->New());

    const google::protobuf::Reflection* reflection = message->GetReflection();
    
    //解析JSON文件并赋值
    Json::Reader reader;
    Json::Value jsonData;
    if(!reader.parse(message_value,jsonData)){
        std::cerr << "Failed to parse json: " << message_value <<std::endl;
        return nullptr;
    }
    for (auto it = jsonData.begin(); it != jsonData.end() ; ++it) {
        const google::protobuf::FieldDescriptor* field = descriptor -> FindFieldByName(it.key().asString());
        if(field == nullptr){
           std::cerr << "Protobuf type(" << message_type << ") is missing the "<< it.key().asString() << "field or has an incorrect type" << std::endl;
            return nullptr;
        }
        reflection -> SetString(message.get(), field, it -> asString());
    }

    return message;
}


void create_publisher(
                const std::string& topic_name,
                const std::string& message_type,
                std::shared_ptr<google::protobuf::Message>& message,
                int rate, bool rate_flag,
                const std::string& message_value)
{
    ////rosbag record功能，根据话题名-时间创建输出文件
    //获取当前时间转化为年月日时分秒的格式
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_c);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d-%H-%M-%S");
    std::string timestamp = oss.str();
    //创建对应的文件夹及当前时间点的文件
    fs::path current_path = fs::current_path(); //当前运行程序的文件夹
    //std::cout << current_path <<std::endl;
    fs::path record_dir = current_path / "src" / "Record";
    fs::path topic_dir = record_dir / topic_name;
    if(!fs::exists(topic_dir)){
        fs::create_directories(topic_dir);
    }
    std::string log_file_dir = (topic_dir / (timestamp + ".txt")).string();

    std::ofstream log_file(log_file_dir); //在第一行记录当前话题发布的数据类型
    if (log_file.is_open()) {
        log_file << "message_type: " << message_type << "\n";
        log_file.close();
    } else {
        std::cerr << "Failed to open log file: " << log_file_dir << std::endl;
    }

    auto pub_node = std::make_shared<Hnu::Middleware::Node>("publish_node");
    if (rate_flag == 0){
        publish_message(pub_node, topic_name, *message, message_type, log_file_dir, message_value);
        raise(SIGINT);
    }
    else{
        auto timer = pub_node->createTimer(rate, [pub_node, topic_name, message, message_type, log_file_dir, message_value]() {
            publish_message(pub_node, topic_name, *message, message_type, log_file_dir, message_value);
        });
    }
    pub_node -> run();
    return;
}

//重播话题消息并且发布给订阅者
void bag_play(const std::string& file_path){
    fs::path real_path = fs::current_path() / "src" / "Record" / file_path;
    std::ifstream infile(real_path);
    if(!infile.is_open()){
        std::cerr << "Failed to open bag file: " << real_path << std::endl;
        return;
    }

    //获取消息类型和话题名
    fs::path path(file_path);
    std::string topic_name = path.parent_path().filename().string();
    //std::cout << "topic: " << topic_name << std::endl;

    std::string line, message_type;
    std::getline(infile, line);
    if(line.find("message_type: ") == 0){
        message_type = line.substr(strlen("message_type: "));
        //std::cout << "message_type: " << message_type << std::endl;
    }
    else {
        std::cerr << "Invalid bag format: no message_type found" << std::endl;
        return;
    }

    //按原有频率发布消息
    auto play_node = std::make_shared<Hnu::Middleware::Node>("bagplay_node");
    auto publisher = play_node->createPublisher<google::protobuf::Message>(topic_name,message_type);
    std::string prev_line;
    double prev_time = 0.0;
    bool first = true;

    while (std::getline(infile,line)) {
        std::istringstream iss(line);
        std::string time_str, json_str;

        //分离时间与消息
        size_t delim_pos = line.find('|');
        if (delim_pos == std::string::npos){
            std::cerr << "Invalid line format: " << line << std::endl;
            continue ;
        }

        time_str = line.substr(0, delim_pos);
        json_str = line.substr(delim_pos+1);

        double current_time = std::stod(time_str);

        //根据两条消息之间的时间差控制发布频率
        if(!first){
            double sleep_time = current_time - prev_time;
            if (sleep_time > 0) {
                std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
            }
        }
        else{
            first = false;
        }

        prev_time = current_time;

        auto message = create_protobuf_message(message_type, json_str);
        if(!message){
            std::cerr << "Failed to create message from: " << json_str << std::endl;
            continue;
        }
        publisher->publish(*message);

        std::cout << "Republishing(" << message->GetTypeName() 
            << ") message to topic " << topic_name 
            << ": " << message->DebugString() << std::endl;
    }
    infile.close();
}

void data_list(const std::string& host, const std::string& target, const std::string& key) {
    //列出所有话题或节点
    send_list_http(host, target, key);
}

void showTopicEcho(/*std::shared_ptr<Hnu::Middleware::Node> node, */
                    const std::string& host, const std::string& target, 
                    const std::string& key, const std::string& topic_name) {
    //获取话题数据类型
    std::string type = send_echo_http(host, target, key, topic_name);
    if(type.empty()){
        return ;
    }
    //创建订阅者并输出话题数据
    subscribe_and_echo(topic_name, type);
}

void publishTopic(/*std::shared_ptr<Hnu::Middleware::Node> node, */
                    const std::string& topic_name, 
                    const std::string& message_type, 
                    const std::string& message_value, 
                    int rate, bool rate_flag) {
    //创建发布者
    auto message = create_protobuf_message(message_type, message_value);
    if(message){
        create_publisher(topic_name, message_type, message, rate, rate_flag, message_value);
    }
}


int main(int argc, char* argv[]){
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
            std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
        }
    }
    else if(tokens[0] == "topic"){
        if(tokens[1] == "echo"){
            if (size == 3){
                if (!tokens[2].empty()) {
                    showTopicEcho(host, echo_target, echo_key,tokens[2]);
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
                    //auto pub_node = std::make_shared<Hnu::Middleware::Node>("publish_node");
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
                    publishTopic(tokens[2], tokens[3], tokens[4], rate, rate_flag);
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
            std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
        }
    }
    else if(tokens[0] == "bag") {
        if (tokens[1] == "play") {
            if (size == 3){
                ////读取对应的文件
                bag_play(tokens[2]);
            }
            else {
                std::cerr << "Error: file_path is required for play" << std::endl;
            }
        }
        else {
            std::cerr << "Invalid command, press [-h] for more detailed usage" << std::endl;
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
        std::cout << "  bag play <file_path(topic_name/Y-M-D-H-M-S.txt)>: Replay information for <topic_name> at time<Y-M-D-H-M-S>" << std::endl;
    }
    else{
        std::cerr << "Unknown command, press [-h] for more detailed usage" <<std::endl;
    }


    return 0;
}
