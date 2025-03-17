#include<hmw/Node.hpp>
#include<iostream>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include<jsoncpp/json/json.h>
#include<hmw/Publisher.hpp>


int main(int argc,char* argv[]){
  if(argc!=6){
    std::cerr<<"Usage: "<<argv[0]<<" topic pub <topic> <type> <msg>"<<std::endl;
    return 1;
  }
  std::string topic=argv[3];
  std::string type=argv[4];
  std::string msg=argv[5];
  Hnu::Middleware::Node node("topicpub");
  const google::protobuf::Descriptor* descriptor =google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type);
  if(descriptor==nullptr){
    std::cerr<<"Type "<<type<<" not found"<<std::endl;
    return 1;
  }
  const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
  google::protobuf::Message* message = prototype->New();
  const google::protobuf::Reflection* reflection = message->GetReflection();


  Json::Reader reader;
  Json::Value root;
  if(!reader.parse(msg,root)){
    std::cerr<<"Failed to parse json: "<<msg<<std::endl;
    return 1;
  }
  for(auto it=root.begin();it!=root.end();++it){
    const google::protobuf::FieldDescriptor* field=descriptor->FindFieldByName(it.key().asString());
    if(field==nullptr){
      std::cerr<<"Field "<<it.key().asString()<<" not found in type "<<type<<std::endl;
      return 1;
    }
    reflection->SetString(message, field, it->asString());
  }
  auto publisher=node.createPublisher<google::protobuf::Message>(topic,type);
  auto timer=node.createTimer(500,[&]{
    publisher->publish(*message);
  });
  node.run();
  

}