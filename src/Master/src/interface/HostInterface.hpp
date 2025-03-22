#pragma once
#include <string>
#include <memory>
#include <boost/asio.hpp>

namespace Hnu::Interface {
  class HostInterface : public std::enable_shared_from_this<HostInterface> {
  public:
    HostInterface(const std::string& name,const std::string& hName, int segment, const std::string& type);
    virtual ~HostInterface() = default;
    // int getSegment() ;
    virtual void run(boost::asio::io_context& ioc)=0;

  protected:
    std::string m_name;
    std::string hostName;
    int m_segment;
    std::string m_type;
  };
}