#include<hmw/Node.hpp>
#include<hmw/Subscriber.hpp>
#include<iostream>
#include<boost/beast.hpp>
#include<jsoncpp/json/json.h>
#include<google/protobuf/message.h>
#include <Std/String.pb.h>

int main(int argc,char* argv[]){
  protobuf_Std_2fString_2eproto::AddDescriptors();
  if(argc!=4){
    std::cerr<<"Usage: "<<argv[0]<<" topic echo <topic> "<<std::endl;
    return 1;
  }
  std::string topic=argv[3];
  boost::asio::io_context ioc;
  boost::asio::local::stream_protocol::socket socket(ioc);
  socket.connect("/tmp/master");
  boost::beast::http::request<boost::beast::http::string_body> req;
  req.set("topic",topic);
  req.method(boost::beast::http::verb::get);
  req.target("/topic/info");
  req.prepare_payload();
  boost::beast::http::write(socket,req);
  boost::beast::http::response<boost::beast::http::string_body> res;
  boost::beast::flat_buffer buffer;
  boost::beast::http::read(socket,buffer,res);
  Json::Reader reader;
  Json::Value root;
  if(!reader.parse(res.body(),root)){
    std::cerr<<"Failed to parse json: "<<res.body()<<std::endl;
    return 1;
  }
  std::string type=root["type"].asString();
  auto node=Hnu::Middleware::Node("topicecho");
  const google::protobuf::Descriptor* descriptor =google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type);
  if(descriptor==nullptr){
    std::cerr<<"Type "<<type<<" not found"<<std::endl;
    return 1;
  }
  // const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
  auto subscriber=node.createSubscriber<google::protobuf::Message>(topic,type,[&](std::shared_ptr<google::protobuf::Message> msg){
    std::cout<<msg->DebugString()<<std::endl;
  });
  node.run();

}