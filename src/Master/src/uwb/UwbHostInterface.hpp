#pragma once

#include "interface/HostInterface.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <queue>
#include <uwb/UwbHandle.hpp>

namespace Hnu::Uwb{
  namespace asio = boost::asio;
  namespace beast = boost::beast;
  class UwbHostInterface:public Hnu::Interface::HostInterface{
  public:
  UwbHostInterface(const std::string& name,const std::string& hName, const std::string& type, int segment, const std::shared_ptr<Handle>& handle);
  void run(asio::io_context& ioc) override;
  void send(beast::http::request<beast::http::string_body>& req) override;
  protected:
    std::shared_ptr<UwbHostInterface> shared_from_this();
  private:
    void doWrite();
    void onWrite(const beast::error_code& ec, std::size_t bytes_transferred);
    std::queue<beast::http::request<beast::http::string_body>> m_requestQueue;
    std::string m_currentWriteData;
    std::shared_ptr<Handle> m_handle;
    bool m_writing = false;
  };
}