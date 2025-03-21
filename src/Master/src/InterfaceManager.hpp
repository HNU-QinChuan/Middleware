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

  private:
    InterfaceManager()=default;
    std::unordered_map<std::string, std::vector<std::string>> topic2host;
    std::unordered_map<std::string, std::pair<std::string, std::string>> route;//desthost interface nexthost
    std::unordered_map<std::string, std::shared_ptr<Host>> hostlist;
    std::unordered_map<std::string, std::shared_ptr<Interface>> interfaceList;
    Map map;
    std::string m_hostName;
  };
}