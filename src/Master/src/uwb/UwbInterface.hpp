#pragma once

#include "interface/Interface.hpp"
#include"uwb/UwbHandle.hpp"


namespace Hnu::Uwb{
  class UwbInterface:public Hnu::Interface::Interface{
  public:
    UwbInterface(const std::string& name,const std::string& type,int segment,const std::string& ip, unsigned baudrate);
    void run(asio::io_context& ioc) override;
    std::shared_ptr<Hnu::Uwb::Handle> getHandle() const;
  private:
    std::string m_device;
    unsigned m_baudrate;
    std::shared_ptr<Hnu::Uwb::Handle> m_handle;
  };
}