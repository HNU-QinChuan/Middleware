//
// Created by yc on 25-3-12.
//
#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include"interface/HostInterface.hpp"

namespace Hnu::Interface{
  class Host:public std::enable_shared_from_this<Host>{
  public:
    Host(const std::string& name);
    void setHostInterface(const std::string& interfaceName, const std::shared_ptr<HostInterface> hostInterface);

  private:
    std::string m_name;
    std::unordered_map<std::string, std::unordered_set<std::string>> node2pubtopic;
    std::unordered_map<std::string, std::unordered_set<std::string>> node2subtopic;

    std::unordered_map<std::string, std::shared_ptr<HostInterface>> hostInterfaceList;
  };
}
