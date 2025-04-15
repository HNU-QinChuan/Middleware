#include "ProcessManager.hpp"
#include"InterfaceManager.hpp"
#include<filesystem>


namespace Hnu::Middleware {
  ProcessManager::ProcessManager(boost::asio::io_context& ioc)
    :m_signalSet(ioc) {
  }
  void ProcessManager::run() {
    std::string execpath=std::filesystem::canonical("/proc/self/exe").parent_path();
    for(auto& exec:Interface::InterfaceManager::interfaceManager.launch){
      std::string execName=exec.asString();
      std::string execPath=execpath+"/"+execName;
      std::shared_ptr<bp::child> process=std::make_shared<bp::child>(execPath);
      m_processes.push_back(process);
    }
    m_signalSet.add(SIGINT);
    m_signalSet.async_wait(std::bind_front(&ProcessManager::onSignal,this));
  }

  void ProcessManager::onSignal(const boost::system::error_code& e,int signal){
    for(auto& process:m_processes){
      process->terminate();
    }
    m_processes.clear();
    std::terminate();
  }

}