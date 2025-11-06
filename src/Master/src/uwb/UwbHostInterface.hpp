#pragma once
#include "interface/HostInterface.hpp"
#include "UwbHandle.hpp"
#include <queue>
#include <atomic>

namespace Hnu::Uwb{
  class UwbHostInterface : public Hnu::Interface::HostInterface {
    public:
      UwbHostInterface(const std::string& name,const std::string& hName, const std::string& type, int segment, const std::shared_ptr<Handle>& handle, uint8_t node_id);
      
      std::shared_ptr<UwbHostInterface> shared_from_this();
      void run(asio::io_context& ioc) override;
      
      // 可靠发送到指定节点
      void send_reliable(beast::http::request<beast::http::string_body>& req, uint8_t target_node, std::function<void(bool)> callback = nullptr);

      // 发送到指定节点
      void send(beast::http::request<beast::http::string_body>& req) override;
      
      // // 保持原有接口（从请求头解析目标节点）
      // void send_all(beast::http::request<beast::http::string_body>& req);
      
    private:
      uint8_t hNode_id; // 主机节点ID
      std::shared_ptr<Handle> m_handle;
      std::queue<beast::http::request<beast::http::string_body>> m_requestQueue;
      std::atomic<bool> m_writing{false};
      std::string m_currentWriteData;
      beast::http::request<beast::http::string_body> request;
      
      void doWrite();
      void onWrite(const beast::error_code& ec, std::size_t bytes_transferred);
      void on_http_received(const beast::http::request<beast::http::string_body>& req, uint8_t from_node);
  };
}