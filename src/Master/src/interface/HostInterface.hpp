#pragma once
#include <string>
#include <memory>


namespace Hnu::Interface {
  class HostInterface : public std::enable_shared_from_this<HostInterface> {
  public:
    HostInterface(const std::string& name, int segment, const std::string& type);
    virtual ~HostInterface() = default;
    // int getSegment() ;

  protected:
    std::string m_name;
    int m_segment;
    std::string m_type;
  };
}