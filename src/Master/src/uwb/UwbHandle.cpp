#include<spdlog/spdlog.h>
#include "uwb/UwbHandle.hpp"
#include"interface/InterfaceRouter.hpp"

namespace Hnu::Uwb {
  Handle::Handle(asio::io_context& ioc, const std::string& device, unsigned baudrate)
    : m_serial(ioc), m_device(device), m_baudrate(baudrate) 
  {
    spdlog::info("UWB Handle created for device: {}, baudrate: {}", device, baudrate);
    boost::system::error_code ec;

    auto _ = m_serial.open(device, ec);
    if (ec) {
      spdlog::error("Failed to open serial device {}: {}", device, ec.message());
      return;
    }

    m_serial.set_option(asio::serial_port_base::baud_rate(baudrate));
    m_serial.set_option(asio::serial_port_base::character_size(8));
    m_serial.set_option(asio::serial_port_base::parity(asio::serial_port_base::parity::none));
    m_serial.set_option(asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one));
    m_serial.set_option(asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none));

    spdlog::info("Serial port {} initialized successfully.", device);
  }

  void Handle::doRead() {
    auto self = shared_from_this();
    beast::http::async_read(m_serial, m_buffer, m_request,
    [self](beast::error_code ec, std::size_t bytes) {
        if (!ec) {
            spdlog::debug("Received HTTP request");
            Interface::InterfaceRouter::handle(self->m_request);
        } else {
            spdlog::error("Failed to read from serial port");
            spdlog::error("HTTP parse error: {}", ec.message());
        }
        self->doRead();  // 继续监听
    });
  }

  void Handle::run() {
    // m_stream.expires_after(std::chrono::seconds(30));
    doRead();
  }
}
