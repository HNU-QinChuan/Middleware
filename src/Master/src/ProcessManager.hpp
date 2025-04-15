#pragma once
#include<boost/process.hpp>
#include<memory>
#include<vector>
#include<boost/asio.hpp>


namespace Hnu::Middleware {
  namespace bp = boost::process;
  class ProcessManager {
  public:  
    ProcessManager(boost::asio::io_context& ioc);
    void run();
  private:
    void onSignal(const boost::system::error_code& e,int signal);
    std::vector<std::shared_ptr<bp::child>> m_processes;
    boost::asio::signal_set m_signalSet;
  };

}