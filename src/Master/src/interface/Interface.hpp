#pragma once

#include <string>
#include<memory>
#include<boost/asio.hpp>
#include <unordered_map>
#include "interface/HostInterface.hpp"

namespace Hnu::Interface{
  namespace asio = boost::asio;
  class Interface:public std::enable_shared_from_this<Interface>{
  public:
    Interface(const std::string& name, const std::string& type, int segment);
    virtual ~Interface() = default;
    int getSegment();
    void setHostInterface(const std::string& name, std::shared_ptr<HostInterface> hostInterface);


    virtual void run() = 0;
    void start();

  protected:
    asio::io_context ioc;
    std::string m_name;
    int m_segment;
    std::string m_type;

    std::unordered_map<std::string, std::shared_ptr<HostInterface>> name2hostInterface;

  };
}