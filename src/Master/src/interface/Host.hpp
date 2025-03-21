//
// Created by yc on 25-3-12.
//
#pragma once

#include <string>
#include <memory>

namespace Hnu::Interface{
  class Host:public std::enable_shared_from_this<Host>{
  public:
    Host(const std::string& name, const std::string& type, int segment)
      : m_name(name), m_type(type), m_segment(segment) {}
    virtual ~Host() = default;


  private:
    std::string m_name;
    std::string m_type;
    int m_segment;
  };
}
