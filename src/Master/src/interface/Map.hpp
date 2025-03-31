#pragma once

#include <string>
#include<forward_list>
#include <unordered_map>
#include <vector>
#include<jsoncpp/json/json.h>


namespace Hnu::Interface {
  struct line{
    int segment=-1;
    int weight=0;
  };
  // struct Node{
  //   std::string name;
  //   Node(const std::string& name):name(name){}
  // };
  class Map{
  public:
    void init(const Json::Value& hosts,const std::string& localhost);
    std::unordered_map<std::string, std::pair<std::string, std::string>> getRoute();
  private:
    std::unordered_map<int, std::string> key2node;
    std::unordered_map<int, std::vector<int>> segment2node;
    std::vector<std::vector<line>> map;
    // std::unordered_map<int, std::string> localSegemnt2interface;
    std::vector<std::unordered_map<int, std::string>> segment2interface;


  };



}