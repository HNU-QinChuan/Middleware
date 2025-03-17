//
// Created by yc on 25-3-12.
//
#pragma once

#include <string>
#include <vector>
#include"interface/Interface.hpp"

namespace Hnu::Interface{
  class Host{
  
  private:
    std::string m_name;
    std::vector<Interface*> m_interfaces;
  };
}
