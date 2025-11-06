#pragma once

#include <boost/asio/steady_timer.hpp>


namespace Hnu::Middleware {
  class Timer:public std::enable_shared_from_this<Timer>{
  public:
    Timer(boost::asio::io_context& io_context);
    void run(const std::function<void()>& callback, std::size_t interval);
    void cancel();
  private:
    void onTime(const boost::system::error_code& error);
    std::function<void()> m_callback;
    std::chrono::milliseconds m_interval;
    boost::asio::steady_timer m_timer;
  };
}