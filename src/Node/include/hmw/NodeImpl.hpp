//
// Created by yc on 25-3-4.
//


#pragma once

#include<boost/asio.hpp>
#include"hmw/PublisherInterface.hpp"
#include"hmw/SubscriberInterface.hpp"
#include<boost/beast.hpp>
namespace Hnu::Middleware {
  namespace asio = boost::asio;
  namespace beast = boost::beast;
  using local_stream = beast::basic_stream<asio::local::stream_protocol>;
  class NodeImpl {
  public:
    NodeImpl(const std::string& name);
    void run();
    bool containsPublisher(const std::string& topic);
    bool containsSubscriber(const std::string& topic);
    void addPublisher(const std::string& topic,std::shared_ptr<PublisherInterface> publisher);
    void addSubscriber(const std::string& topic,std::shared_ptr<SubscriberInterface> subscriber);
    std::shared_ptr<SubscriberInterface> getSubscriber(const std::string& topic);
    std::shared_ptr<PublisherInterface> getPublisher(const std::string& topic);
    std::string getName();

  private:
    void onSignal(const boost::system::error_code& ec,int signal);

    asio::io_context m_ioc;
    local_stream m_socket;
    std::string m_name;
    pid_t m_pid;
    asio::signal_set m_signalSet;
    std::unordered_map<std::string,std::shared_ptr<PublisherInterface>> m_publishes;
    std::unordered_map<std::string,std::shared_ptr<SubscriberInterface>> m_subscribes;
  };
}




