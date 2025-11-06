
#include "tcp/Acceptor.hpp"
#include "tcp/Server.hpp"
#include<spdlog/spdlog.h>

// #include "hilog/log.h"
// #undef LOG_DOMAIN
// #undef LOG_TAG
// #define LOG_DOMAIN 0x3200
// #define LOG_TAG "MasterService"

namespace Hnu::Tcp {
//   Acceptor::Acceptor(asio::io_context& ioc,const std::string& ip, unsigned port): 
//   m_endpoint(asio::ip::make_address(ip), port), m_acceptor(ioc, m_endpoint) {
//
//   }
    Acceptor::Acceptor(asio::io_context &ioc, const std::string &ip, unsigned port)
        : m_acceptor(ioc), m_endpoint(asio::ip::make_address(ip), port) {
        boost::system::error_code ec;

        m_acceptor.open(m_endpoint.protocol(), ec);
        if (ec) {
            //OH_LOG_ERROR(LOG_APP, "【MasterService】Acceptor open failed: %{public}s", ec.message().c_str());
            throw boost::system::system_error(ec);
        }

        m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);
        if (ec) {
            //OH_LOG_WARN(LOG_APP, "【MasterService】Set reuse_address failed: %{public}s", ec.message().c_str());
        // 不抛异常，非致命错误
        }

        m_acceptor.bind(m_endpoint, ec);
        if (ec) {
            //OH_LOG_ERROR(LOG_APP, "【MasterService】Acceptor bind failed: %{public}s", ec.message().c_str());
            //OH_LOG_ERROR(LOG_APP, "【MasterService】Address: %{public}s:%{public}u", ip.c_str(), port);
            throw boost::system::system_error(ec);
        }

        m_acceptor.listen(asio::socket_base::max_listen_connections, ec);
        if (ec) {
            //OH_LOG_ERROR(LOG_APP, "【MasterService】Acceptor listen failed: %{public}s", ec.message().c_str());
            throw boost::system::system_error(ec);
        }

        //OH_LOG_INFO(LOG_APP, "【MasterService】Acceptor is listening on %{public}s:%{public}u", ip.c_str(), port);
    }
  void Acceptor::run() {
    doAccept();
  }
  void Acceptor::doAccept() {
    auto server=std::make_shared<Server>(m_acceptor.get_executor().context());
    m_acceptor.async_accept(server->socket(),std::bind_front(&Acceptor::onAccept, this, server));
    //m_acceptor.async_accept(server->socket(), std::bind(&Acceptor::onAccept, this, server, std::placeholders::_1));
  }
  void Acceptor::onAccept(std::shared_ptr<Server> acceptServer, const boost::system::error_code& ec) {
    doAccept();
    if (ec) {
      spdlog::error("Accept Error from tcp: {}", ec.message());
      return;
    }
    acceptServer->run();
  }
}