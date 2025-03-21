#include "interface/HostInterface.hpp"


namespace Hnu::Interface {
  HostInterface::HostInterface(const std::string& name, int segment, const std::string& type)
    : m_name(name), m_segment(segment), m_type(type) {}
  // int HostInterface::getSegment() {
  //   return m_segment;
  // }
}