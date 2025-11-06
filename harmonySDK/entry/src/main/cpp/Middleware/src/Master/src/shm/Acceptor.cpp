//
// Created by yc on 25-2-14.
//

#include "shm/Acceptor.hpp"
#include "shm/Server.hpp"
#include<spdlog/spdlog.h>
#include<filesystem>
#include "MiddlewareManager.hpp"


namespace Hnu::Middleware {
  //Acceptor::Acceptor(asio::io_context& ioc):m_acceptor(ioc),m_endpoint("/data/storage/el2/base/files/master.sock")
//   Acceptor::Acceptor(asio::io_context &ioc) : m_acceptor(ioc), m_endpoint("/data/local/tmp/master.sock") {
//       spdlog::debug("Acceptor Created");
//   }
  Acceptor::Acceptor(asio::io_context &ioc, unsigned short port)
    : m_acceptor(ioc), m_endpoint(asio::ip::tcp::v4(), port) {
    spdlog::debug("Acceptor Created on port {}", port);
  }

//   void Acceptor::run() {
//     spdlog::info("Doing Accept on {}", m_endpoint.path());
//     //std::filesystem::remove("/data/storage/el2/base/files/master.sock");
//     std::filesystem::remove("/data/local/tmp/master.sock");
//     m_acceptor.open(asio::local::stream_protocol());
//     m_acceptor.bind(m_endpoint);
//     m_acceptor.listen(asio::socket_base::max_listen_connections);
//     doAccept();
//     // m_ioc.run();
//   }
//   void Acceptor::run() {
//     try {
//       m_acceptor.open(m_endpoint.protocol());
//       m_acceptor.set_option(asio::socket_base::reuse_address(true));  // 允许地址复用
//       m_acceptor.bind(m_endpoint);
//       m_acceptor.listen(asio::socket_base::max_listen_connections);
//       spdlog::info("TCP Acceptor is listening on port {}", m_endpoint.port());
//       doAccept();
//     } catch (const std::exception& e) {
//       spdlog::error("Acceptor run error: {}", e.what());
//     }
//   }
void Acceptor::run() {
    spdlog::info("Doing Accept on port {}", m_endpoint.port());
    
    boost::system::error_code ec;
    if (!m_acceptor.is_open()) {
        m_acceptor.open(asio::ip::tcp::v4(), ec);
        if (ec) {
            spdlog::error("Acceptor open error: {}", ec.message());
            return;
        }
    }

    m_acceptor.set_option(asio::socket_base::reuse_address(true), ec);
    if (ec) {
        spdlog::error("Set reuse_address error: {}", ec.message());
        return;
    }

    m_acceptor.bind(m_endpoint, ec);
    if (ec) {
        spdlog::error("Acceptor bind error: {}", ec.message());
        return;
    }

    m_acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        spdlog::error("Acceptor listen error: {}", ec.message());
        return;
    }

    doAccept();
}


  void Acceptor::doAccept() {
    //auto server=std::make_shared<Server>(m_acceptor.get_executor().context());
    auto server = std::make_shared<Server>(m_acceptor.get_executor().context());
    auto& sock = server->socket();
    m_acceptor.async_accept(sock,
    [this, server](const boost::system::error_code& ec) {
        this->onAccept(server, ec);
    });
    //m_acceptor.async_accept(server->socket(),std::bind_front(&Acceptor::onAccept, this, server));
    //m_acceptor.async_accept(server->socket(), std::bind(&Acceptor::onAccept, this, server, std::placeholders::_1));
  }
  void Acceptor::onAccept(std::shared_ptr<Server> acceptServer, const boost::system::error_code& ec) {
    doAccept();
    if (ec) {
      spdlog::error("Accept Error from shm: {}", ec.message());
      return;
    }
    acceptServer->run();
  }
}