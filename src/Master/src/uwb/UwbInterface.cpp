#include "uwb/UwbInterface.hpp"
#include "spdlog/spdlog.h"


namespace Hnu::Uwb{
  UwbInterface::UwbInterface(const std::string& name,const std::string& type,int segment,const std::string& device, unsigned baudrate)
    : Hnu::Interface::Interface(name, type, segment), m_device(device), m_baudrate(baudrate) {
      
  }
  
  void UwbInterface::run(asio::io_context& ioc){
    if (m_handle) {
        spdlog::warn("UwbInterface::run() called more than once, ignoring.");
        return;
    }
    m_handle = std::make_shared<Hnu::Uwb::Handle >(ioc, m_device, m_baudrate);
    m_handle->run();
}
  
  std::shared_ptr<Hnu::Uwb::Handle> UwbInterface::getHandle() const {
    return m_handle;
  }
}