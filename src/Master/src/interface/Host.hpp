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
    void addNode(const std::string& node);
    void deleteNode(const std::string& node);
    void addSub(const std::string& node, const std::string& topic, const std::string& type);
    void deleteSub(const std::string& node, const std::string& topic);
    void addPub(const std::string& node, const std::string& topic, const std::string& type);
    void deletePub(const std::string& node, const std::string& topic);

  private:
    std::string m_name;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> node2pubtopic2type;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> node2subtopic2type;
    std::unordered_set<std::string> nodelist;

    std::unordered_map<std::string, std::shared_ptr<HostInterface>> hostInterfaceList;
  };
}
