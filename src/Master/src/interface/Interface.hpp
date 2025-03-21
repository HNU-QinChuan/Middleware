#pragma once

#include <string>
#include<memory>
#include <vector>
#include <unordered_map>
#include "interface/Host.hpp"

namespace Hnu::Interface{
  class Interface:public std::enable_shared_from_this<Interface>{
  public:
    Interface(const std::string& name, const std::string& type, int segment)
      : m_name(name), m_type(type), m_segment(segment) {}
    virtual ~Interface() = default;
    int getSegment() const { return m_segment; }

    void insertHost(const std::string& interfaceName,const std::string&hostName, std::shared_ptr<Host> host) {
      interface2host[interfaceName][hostName] = host;
    }

  private:
    std::string m_name;
    int m_segment;
    std::string m_type;
    std::unordered_map<std::string,std::unordered_map<std::string, std::shared_ptr<Host>>> interface2host;

  };
}