#include "interface/Host.hpp"



namespace Hnu::Interface {
  Host::Host(const std::string& name) : m_name(name) {
   
  }

  void Host::setHostInterface(const std::string& interfaceName, const std::shared_ptr<HostInterface> hostInterface) {
    hostInterfaceList[interfaceName] = hostInterface;
  }
  void Host::addNode(const std::string& node) {
    nodelist.insert(node);
  }
  std::vector<std::string> Host::deleteNode(const std::string& node) {
    std::vector<std::string> res;
    nodelist.erase(node);
    node2pubtopic2type.erase(node);
    for(auto& [nodeName,topic2type]:node2subtopic2type){
      for(auto& [topic,type]:topic2type){
        res.push_back(topic);
      }
    }
    node2subtopic2type.erase(node);
    return res;
  }
  void Host::addSub(const std::string& node, const std::string& topic, const std::string& type) {
    node2subtopic2type[node][topic] = type;
  }
  void Host::deleteSub(const std::string& node, const std::string& topic) {
    node2subtopic2type[node].erase(topic);
  }
  void Host::addPub(const std::string& node, const std::string& topic, const std::string& type) {
    node2pubtopic2type[node][topic] = type;
  }
  void Host::deletePub(const std::string& node, const std::string& topic) {
    node2pubtopic2type[node].erase(topic);
  }
  void Host::clear(){
    nodelist.clear();
    node2pubtopic2type.clear();
    node2subtopic2type.clear();
  }
}