#pragma once

#include "interface/Host.hpp"
#include "interface/Interface.hpp"
#include "interface/Map.hpp"
#include<unordered_map>
#include <vector>
#include <memory>


namespace Hnu::Interface {
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

  private:
    InterfaceManager()=default;
    std::unordered_map<std::string, std::vector<std::string>> topic2host;//topic对host的映射
    std::unordered_map<std::string, std::pair<std::string, std::string>> route;//desthost interface nextinterface
    std::unordered_map<std::string, std::shared_ptr<Host>> hostlist;
    std::unordered_map<std::string, std::shared_ptr<Interface>> interfaceList;
    Map map;
    std::string m_hostName;
    std::vector<std::thread*> threads;
  };
}