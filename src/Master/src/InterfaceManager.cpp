#include "InterfaceManager.hpp"
#include <jsoncpp/json/json.h>
#include <spdlog/spdlog.h>
#include<filesystem>
#include <fstream>
#include "tcp/TcpInterface.hpp"
#include "tcp/TcpHost.hpp"

namespace Hnu::Interface {

  // InterfaceManager& InterfaceManager::getInstance() {
  //   static InterfaceManager instance; 
  //   return instance;                
  // }
  InterfaceManager InterfaceManager::interfaceManager;

  void InterfaceManager::init(const std::string& hostName){
    m_hostName = hostName;
    std::string execpath=std::filesystem::canonical("/proc/self/exe").parent_path();
    std::string configPath = execpath+"./config.json";
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
            int port = interface["port"].asInt();
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
        const Json::Value& interfaces = host["interfaces"];
        for (const auto& interface : interfaces) {
          std::string interfaceName = interface["name"].asString();
          std::string type = interface["type"].asString();
          int segment = interface["segment"].asInt();
          std::shared_ptr<Host> hostInstance;
          if(type=="tcp"){
            std::string ip = interface["ip"].asString();
            int port = interface["port"].asInt();
            hostInstance = std::make_shared<Tcp::TcpHost>(interfaceName, type, segment, ip, port);
          }
          hostlist[hostName] = hostInstance;
          for(auto&[key,value]:interfaceList){
            if(value->getSegment()==segment){
              value->insertHost(key, hostName, hostInstance);
            }
          }
        }
      }
    }



  }
}