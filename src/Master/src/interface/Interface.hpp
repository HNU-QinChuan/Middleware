#pragma once

#include <string>

namespace Hnu::Interface{
  class Interface{
  public:
    Interface() = default;
    virtual ~Interface() = default;


  private:
    std::string m_name;

  

  };
}