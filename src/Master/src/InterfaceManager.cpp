#include "InterfaceManager.hpp"

#include <spdlog/spdlog.h>
#include<filesystem>
#include <fstream>
#include "tcp/TcpInterface.hpp"
#include "tcp/TcpHostInterface.hpp"
#include "uwb/UwbInterface.hpp"
#include "uwb/UwbHostInterface.hpp"
#include "MiddlewareManager.hpp"

namespace Hnu::Interface {

  // InterfaceManager& InterfaceManager::getInstance() {
  //   static InterfaceManager instance; 
  //   return instance;                
  // }
  InterfaceManager InterfaceManager::interfaceManager;

  void InterfaceManager::init(const std::string& hostName){
    m_hostName = hostName;
    std::string execpath=std::filesystem::canonical("/proc/self/exe").parent_path();
    std::string configPath = execpath+"/masterconfig.json";
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
      spdlog::error("Failed to open config file: {}", configPath);
      std::terminate();
    }
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    if (!Json::parseFromStream(reader, configFile, &root, &errs)) {
      spdlog::error("Failed to parse config file: {}", errs);
      std::terminate();
    }
    const Json::Value& hosts = root["hosts"];
    for (const auto& host : hosts) {
      std::string hostName = host["name"].asString();
      if(hostName==m_hostName){
        spdlog::info("Host name: {}", hostName);
        const Json::Value& interfaces = host["interfaces"];
        for (const auto& interface : interfaces) {
          std::string interfaceName = interface["name"].asString();
          std::string type = interface["type"].asString();
          int segment = interface["segment"].asInt();
          std::shared_ptr<Interface> interfacePtr;
          if(type=="tcp"){
            std::string ip = interface["ip"].asString();
            unsigned port = interface["port"].asUInt();
            interfacePtr = std::make_shared<Tcp::TcpInterface>(interfaceName, type, segment, ip, port); 
          }
          else if(type=="uwb"){
            //添加初始化
            std::string device = interface["device"].asString();
            unsigned baudrate = interface["baudrate"].asUInt();
            interfacePtr = std::make_shared<Uwb::UwbInterface>(interfaceName, type, segment, device, baudrate);
            auto uwbInterface = std::dynamic_pointer_cast<Uwb::UwbInterface>(interfacePtr);
            m_uwbHandle = uwbInterface->getHandle();
          }
          interfaceList[interfaceName] = interfacePtr;
        }
        break;  
      }
    }
    for(const auto& host : hosts) {
      std::string hostName = host["name"].asString();
      if(hostName!=m_hostName){
        std::shared_ptr<Host> hostInstance=std::make_shared<Host>(hostName);
        const Json::Value& interfaces = host["interfaces"];
        for (const auto& interface : interfaces) {
          std::string interfaceName = interface["name"].asString();
          std::string type = interface["type"].asString();
          int segment = interface["segment"].asInt();
          std::shared_ptr<HostInterface> hostInterface;
          if(type=="tcp"){
            std::string ip = interface["ip"].asString();
            unsigned port = interface["port"].asUInt();
            hostInterface = std::make_shared<Tcp::TcpHostInterface>(interfaceName,hostName, type, segment, ip, port);
          }
          else if(type=="uwb"){
            //添加初始化
            hostInterface = std::make_shared<Uwb::UwbHostInterface>(interfaceName,hostName, type, segment, m_uwbHandle);
          }
          hostInstance->setHostInterface(interfaceName, hostInterface);
          for(auto& [key,value]:interfaceList){
            if(value->getSegment()==segment){
              // spdlog::debug("same segment {} {}",value->getSegment(),segment);
              value->setHostInterface(interfaceName, hostInterface);
            }
          }
        }
        hostlist[hostName] = hostInstance;
      }
    }
    map.init(root, m_hostName);
    route = map.getRoute();
    launch = root["launch"];
  }
  void InterfaceManager::run(){
    for(const auto&[key,value]:interfaceList){
      value->start(Middleware::MiddlewareManager::getIoc());
    }
  }

  void InterfaceManager::broadcast(http::request<http::string_body>& req){
    req.set("src",InterfaceManager::interfaceManager.m_hostName);
    // spdlog::debug("Broadcast to all hosts");
    for(auto&[host,hostptr]:InterfaceManager::interfaceManager.hostlist){
      req.set("dest",host);
      std::string interfaceName=InterfaceManager::interfaceManager.route[host].first;
      std::string nextInterface=InterfaceManager::interfaceManager.route[host].second;
      // spdlog::debug("Broadcast to {} via {}:{}",host,interfaceName,nextInterface);
      InterfaceManager::interfaceManager.interfaceList[interfaceName]->send(nextInterface,req);
    }
  }

  void InterfaceManager::transfer(const std::string& dest,http::request<http::string_body>& req){
    std::string interfaceName=InterfaceManager::interfaceManager.route[dest].first;
    std::string nextInterface=InterfaceManager::interfaceManager.route[dest].second;
    InterfaceManager::interfaceManager.interfaceList[interfaceName]->send(nextInterface,req);
  }
  void InterfaceManager::addNode(const std::string &node){
    http::request<http::string_body> req{
      http::verb::post,
      "/node",
      11
    };
    req.set("node",node);
    broadcast(req);
  }
  void InterfaceManager::deleteNode(const std::string &node){
    http::request<http::string_body> req{
      http::verb::delete_,
      "/node",
      11
    };
    req.set("node",node);
    broadcast(req);
  }
  void InterfaceManager::addSub(const std::string &node, const std::string &topic, const std::string &type){
    http::request<http::string_body> req{
      http::verb::post,
      "/node/sub",
      11
    };
    req.set("node",node);
    req.set("sub",topic);
    req.set("type",type);
    broadcast(req);
  }
  void InterfaceManager::deleteSub(const std::string &node, const std::string &topic){
    http::request<http::string_body> req{
      http::verb::delete_,
      "/node/sub",
      11
    };
    req.set("node",node);
    req.set("sub",topic);
    broadcast(req);
  }
  void InterfaceManager::addPub(const std::string &node, const std::string &topic, const std::string &type){
    http::request<http::string_body> req{
      http::verb::post,
      "/node/pub",
      11
    };
    req.set("node",node);
    req.set("pub",topic);
    req.set("type",type);
    broadcast(req);
  }
  void InterfaceManager::deletePub(const std::string &node, const std::string &topic){
    http::request<http::string_body> req{
      http::verb::delete_,
      "/node/pub",
      11
    };
    req.set("node",node);
    req.set("pub",topic);
    broadcast(req);
  }
  void InterfaceManager::publish(const std::string &topic, const std::string &data){
    http::request<http::string_body> req{
      http::verb::post,
      "/publish",
      11
    };
    req.set("topic",topic);
    req.body()=data;
    // spdlog::debug("publist topic {} data {}",topic,data);
    auto& host=InterfaceManager::interfaceManager.topic2host[topic];
    req.set("src",InterfaceManager::interfaceManager.m_hostName);
    for(auto& hostName:host){
      // spdlog::debug("publish to host {}",hostName);
      req.set("dest",hostName);
      std::string interfaceName=InterfaceManager::interfaceManager.route[hostName].first;
      std::string nextInterface=InterfaceManager::interfaceManager.route[hostName].second;
      InterfaceManager::interfaceManager.interfaceList[interfaceName]->send(nextInterface,req);
    }

  }
  void InterfaceManager::setMaxWeight(const std::string &host){
    interfaceManager.map.setMaxWeight(interfaceManager.m_hostName,host);
    interfaceManager.route=InterfaceManager::interfaceManager.map.getRoute();
    http::request<http::string_body> req{
      http::verb::post,
      "/weight",
      11
    };
    req.set("host1",interfaceManager.m_hostName);
    req.set("host2",host);
    req.set("type","max");
    broadcast(req);
  }
  void InterfaceManager::setInitWeight(const std::string &host){
    InterfaceManager::interfaceManager.map.setInitWeight(interfaceManager.m_hostName,host);
    InterfaceManager::interfaceManager.route=InterfaceManager::interfaceManager.map.getRoute();
    http::request<http::string_body> req{
      http::verb::post,
      "/weight",
      11
    };
    req.set("host1",interfaceManager.m_hostName);
    req.set("host2",host);
    req.set("type","init");
    broadcast(req);
  }
  void InterfaceManager::setMaxWeight(const std::string &host1, const std::string &host2){
    interfaceManager.map.setMaxWeight(host1,host2);
    interfaceManager.route=InterfaceManager::interfaceManager.map.getRoute();
  }
  void InterfaceManager::setInitWeight(const std::string &host1, const std::string &host2){
    interfaceManager.map.setInitWeight(host1,host2);
    interfaceManager.route=InterfaceManager::interfaceManager.map.getRoute();
  }
}