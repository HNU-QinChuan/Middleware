#pragma once

#include "interface/Host.hpp"
#include "interface/Interface.hpp"
#include "interface/Map.hpp"
#include<unordered_map>
#include <unordered_set>
#include <memory>
#include <jsoncpp/json/json.h>
#include<boost/beast.hpp>


namespace Hnu::Interface {
  namespace http = boost::beast::http;
  class InterfaceManager {
  public:
    InterfaceManager(const InterfaceManager&) = delete;
    InterfaceManager& operator=(const InterfaceManager&) = delete;
    InterfaceManager(InterfaceManager&&) = delete;
    InterfaceManager& operator=(InterfaceManager&&) = delete;
    // static InterfaceManager& getInstance();
    static InterfaceManager interfaceManager;
    void init(const std::string& hostName);
    void run();
    static void broadcast(http::request<http::string_body>& req);
    static void transfer(const std::string&dest,http::request<http::string_body>& req);
    static void addNode(const std::string& node);
    static void deleteNode(const std::string& node);
    static void addSub(const std::string& node, const std::string& topic, const std::string& type);
    static void deleteSub(const std::string& node, const std::string& topic);
    static void addPub(const std::string& node, const std::string& topic, const std::string& type);
    static void deletePub(const std::string& node, const std::string& topic);
    static void publish(const std::string& topic, const std::string& data);
    std::string m_hostName;
    std::unordered_map<std::string, std::unordered_set<std::string>> topic2host;//topic对host的映射
    std::unordered_map<std::string, std::pair<std::string, std::string>> route;//desthost interface nextinterface
    std::unordered_map<std::string, std::shared_ptr<Host>> hostlist;
    std::unordered_map<std::string, std::shared_ptr<Interface>> interfaceList;
    Json::Value launch{Json::arrayValue};
  private:
    InterfaceManager()=default;

    Map map;

  };
}