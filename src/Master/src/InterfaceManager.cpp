#include "InterfaceManager.hpp"
#include <jsoncpp/json/json.h>
#include <spdlog/spdlog.h>
#include<filesystem>
#include <fstream>
#include "tcp/TcpInterface.hpp"
#include "tcp/TcpHostInterface.hpp"
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
          if(type=="tcp"){
            std::string ip = interface["ip"].asString();
            unsigned port = interface["port"].asUInt();
            auto tcpInterface = std::make_shared<Tcp::TcpInterface>(interfaceName, type, segment, ip, port);
            interfaceList[interfaceName] = tcpInterface;
          }
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
          hostInstance->setHostInterface(interfaceName, hostInterface);
          for(auto& [key,value]:interfaceList){
            if(value->getSegment()==segment){
              value->setHostInterface(interfaceName, hostInterface);
            }
          }
        }
      }
    }
    map.init(hosts, m_hostName);
    route = map.getRoute();
    for (const auto& [key, value] : route) {
      spdlog::debug("Route: {} -> {} -> {}", key, value.first, value.second);
    }
  }
  void InterfaceManager::run(){
    iocTread = new std::thread([this]() {
      asio::io_context::work work(ioc);
      ioc.run();
    });
    for(const auto&[key,value]:interfaceList){
      std::thread* thread = new std::thread([value]() {
        value->start();
      });
      threads.push_back(thread);
    }
  }
  void InterfaceManager::newConnectRun(){
    Middleware::MiddlewareManager::getIoc().
    post(std::bind(Middleware::MiddlewareManager::getAllNodeInfo,std::ref(interfaceManager.ioc),
    &InterfaceManager::broadcast));
  }
  void InterfaceManager::broadcast(std::string jsonString){
    
  }
}