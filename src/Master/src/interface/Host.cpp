#include "interface/Host.hpp"



namespace Hnu::Interface {
  Host::Host(const std::string& name) : m_name(name) {
   
  }

  void Host::setHostInterface(const std::string& interfaceName, const std::shared_ptr<HostInterface> hostInterface) {
    hostInterfaceList[interfaceName] = hostInterface;
  }
}