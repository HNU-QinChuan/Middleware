#include "interface/Interface.hpp"


namespace Hnu::Interface {
  Interface::Interface(const std::string& name, const std::string& type, int segment)
    : m_name(name), m_type(type), m_segment(segment) {}



  int Interface::getSegment()  { return m_segment; }

  void Interface::setHostInterface(const std::string& name, std::shared_ptr<HostInterface> hostInterface) {
    name2hostInterface[name] = hostInterface;
  }

  void Interface::start(){
    run();
    for(auto&[key,value]:name2hostInterface){
      value->run(ioc);
    }

    asio::io_context::work work(ioc);
    ioc.run();
  }
}