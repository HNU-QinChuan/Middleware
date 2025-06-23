#include <uwb/UwbHostInterface.hpp>
#include <spdlog/spdlog.h>

namespace Hnu::Uwb{

  UwbHostInterface::UwbHostInterface(const std::string& name,const std::string& hName, const std::string& type, int segment, const std::shared_ptr<Handle>& handle)
    : Hnu::Interface::HostInterface(name, hName, segment, type), m_handle(handle)
  {
  }
  std::shared_ptr<UwbHostInterface> UwbHostInterface::shared_from_this() {
    return std::static_pointer_cast<UwbHostInterface>(Hnu::Interface::HostInterface::shared_from_this());
  }
  void UwbHostInterface::run(asio::io_context& ioc) {
  }
  void UwbHostInterface::send(beast::http::request<beast::http::string_body>& req) {
    req.prepare_payload();
    m_requestQueue.push(req);
    doWrite();
  }

  void UwbHostInterface::doWrite() {
    if (m_writing || m_requestQueue.empty()) {
        return;
    }

    m_writing = true;
    request = m_requestQueue.front();  // 保存当前写入请求
    m_requestQueue.pop();

    // 转为字符串发送
    std::stringstream ss;
    ss << request;
    m_currentWriteData = ss.str();

    auto self = shared_from_this();
    asio::async_write(
        m_handle->m_serial,
        asio::buffer(m_currentWriteData),
        [this, self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            onWrite(ec, bytes_transferred);
        }
    );
  }

  void UwbHostInterface::onWrite(const beast::error_code& ec, std::size_t bytes_transferred) {
    m_writing = false;
    if (ec) {
        spdlog::error("Serial write error: {}", ec.message());
        return;
    }
    // 写成功后，如果还有排队请求，继续写
    doWrite();
  }
}