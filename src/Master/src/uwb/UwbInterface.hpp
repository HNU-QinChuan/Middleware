#pragma once
#include "interface/Interface.hpp"
#include "UwbHandle.hpp"

namespace Hnu::Uwb{
  class UwbInterface : public Hnu::Interface::Interface {
    public:
      // 新构造函数（带node_id）
      UwbInterface(const std::string& name,const std::string& type,int segment,const std::string& device, unsigned baudrate, uint8_t node_id);
      
      // 原构造函数（向后兼容，默认node_id=1）
      UwbInterface(const std::string& name,const std::string& type,int segment,const std::string& device, unsigned baudrate);
      
      void run(asio::io_context& ioc) override;
      std::shared_ptr<Hnu::Uwb::Handle> getHandle() const;

      void broadcast(beast::http::request<beast::http::string_body>& req);
      
      // 新增方法
      uint8_t getNodeId() const { return m_node_id; }
      Handle::Stats getStats() const;
      
    private:
      std::string m_device;
      unsigned m_baudrate;
      uint8_t m_node_id = 1; // 默认节点ID
      std::shared_ptr<Hnu::Uwb::Handle> m_handle;
      std::string m_type = "uwb";
  };
}