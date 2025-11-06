#include "hmw/Timer.hpp"


namespace Hnu::Middleware {
  Timer::Timer(boost::asio::io_context& io_context) : m_timer(io_context) {
    
  }
  void Timer::run(const std::function<void()>& callback, std::size_t interval) {
    m_callback = callback;
    m_interval = std::chrono::milliseconds(interval);
    m_timer.expires_after(m_interval);
    m_timer.async_wait(std::bind_front(&Timer::onTime, shared_from_this()));
  }
  void Timer::onTime(const boost::system::error_code& error) {
    if (error) {
      return;
    }
    m_callback();
    m_timer.expires_after(m_interval);
    m_timer.async_wait(std::bind_front(&Timer::onTime, shared_from_this()));
  }
  void Timer::cancel() {
    m_timer.cancel();
  }
}