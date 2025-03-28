#include "interface/Host.hpp"



namespace Hnu::Interface {
  Host::Host(const std::string& name) : m_name(name) {
   
  }

  void Host::setHostInterface(const std::string& interfaceName, const std::shared_ptr<HostInterface> hostInterface) {
    hostInterfaceList[interfaceName] = hostInterface;
  }
  void Host::addNode(const std::string& node) {
    nodelist[node] = node;
  }
  void Host::addSub(const std::string& node, const std::string& topic, const std::string& type) {
    node2subtopic2type[node][topic] = type;
  }
  void Host::addPub(const std::string& node, const std::string& topic, const std::string& type) {
    node2pubtopic2type[node][topic] = type;
  }
}