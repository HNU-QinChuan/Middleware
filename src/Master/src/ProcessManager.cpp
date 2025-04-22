#include "ProcessManager.hpp"
#include"InterfaceManager.hpp"
#include<filesystem>
#include<spdlog/spdlog.h>


namespace Hnu::Middleware {
  ProcessManager::ProcessManager()
    :m_signalSet(ioc) {
  }
  void ProcessManager::run() {
    std::string execpath=std::filesystem::canonical("/proc/self/exe").parent_path();
    for(auto& exec:Interface::InterfaceManager::interfaceManager.launch){
      std::string execName=exec.asString();
      std::string execPath=execpath+"/"+execName;
      std::shared_ptr<bp::child> process=std::make_shared<bp::child>(execPath);
      if(!process->running()){
        spdlog::error("process {} not running",execPath);
        throw std::runtime_error("process not running");
      }
      m_execs.push_back(execPath);
      m_processes.push_back(process);
    }
    m_signalSet.add(SIGINT);
    m_signalSet.add(SIGCHLD);
    m_signalSet.async_wait(std::bind_front(&ProcessManager::onSignal,this));
    std::thread{
      [&]{
        ioc.run();
      }
    }.detach();
  }

  void ProcessManager::onSignal(const boost::system::error_code& e,int signal){
    if(e||signal==SIGINT){
      for(auto& process:m_processes){
        // kill(process->native_handle(), SIGINT);
        process->wait();
      }
      std::exit(0);
    }else if(signal==SIGCHLD){
      for(int i=0;i<m_processes.size();++i){
        if(!m_processes[i]->running()){
          m_processes[i]->wait();
          m_processes[i]=std::make_shared<bp::child>(m_execs[i]);
          if(!m_processes[i]->running()){
            spdlog::error("process {} not running",m_execs[i]);
            throw std::runtime_error("process not running");
          }
        }
      }
    }
  }

}