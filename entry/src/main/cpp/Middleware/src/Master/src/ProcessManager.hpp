#pragma once
#include<boost/process.hpp>
#include<memory>
#include<vector>
#include<boost/asio.hpp>


namespace Hnu::Middleware {
  namespace bp = boost::process;
  class ProcessManager {
  public:  
    ProcessManager();
    void run();
  private:
    void onSignal(const boost::system::error_code& e,int signal);
    boost::asio::io_context ioc;
    std::vector<std::shared_ptr<bp::child>> m_processes;
    std::vector<std::string> m_execs;
    boost::asio::signal_set m_signalSet;

  };

}