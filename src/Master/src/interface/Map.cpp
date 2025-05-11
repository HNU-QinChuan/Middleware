#include "interface/Map.hpp"
#include<spdlog/spdlog.h>
#include<climits>

namespace Hnu::Interface {

  void Map::init(const Json::Value& root,const std::string& localhost) {
    auto hosts=root["hosts"];
    // std::unordered_map<int, std::string> localSegemnt2interface;
    for (const auto& host : hosts) {
      std::string hostName = host["name"].asString();
      if(hostName==localhost){
        key2node[0]=hostName;
        node2key[hostName]=0;
        const Json::Value& interfaces = host["interfaces"];
        std::unordered_map<int, std::string> localSegemnt2interface;
        for (const auto& interface : interfaces) {
          std::string interfaceName = interface["name"].asString();
          int segment = interface["segment"].asInt();
          localSegemnt2interface[segment]=interfaceName;
          segment2node[segment].push_back(0);
        }
        segment2interface.push_back(localSegemnt2interface);
        break;  
      }

    }
    int i=0;
    for (const auto& host : hosts) {
      std::string hostName = host["name"].asString();
      if(hostName!=localhost){
        i++;
        key2node[i]=hostName;
        node2key[hostName]=i;
        const Json::Value& interfaces = host["interfaces"];
        std::unordered_map<int, std::string> s2i;
        for (const auto& interface : interfaces) {
          std::string interfaceName = interface["name"].asString();
          int segment = interface["segment"].asInt();
          s2i[segment]=interfaceName;
          segment2node[segment].push_back(i);
        }
        segment2interface.push_back(s2i);
      }
    }
    map.resize(i+1);
    for(int j=0;j<i+1;++j){
      map[j].resize(i+1);
    }
    for(auto& [key,value]:segment2node){
      for(int j=0;j<value.size();++j){
        for(int k=j+1;k<value.size();++k){
          map[value[j]][value[k]].segment=key;
          map[value[k]][value[j]].segment=key;
          map[value[j]][value[k]].weight=1;
          map[value[k]][value[j]].weight=1;
          map[value[j]][value[k]].initWeight=1;
          map[value[k]][value[j]].initWeight=1;
        }
      }
    }
    auto maps=root["map"];
    for (const auto& mapItem : maps) {
      std::string src = mapItem[0].asString();
      std::string dest = mapItem[1].asString();
      int weight = mapItem[2].asInt();
      int srcIndex=node2key[src];
      int destIndex=node2key[dest];
      map[srcIndex][destIndex].weight=weight;
      map[destIndex][srcIndex].weight=weight;
      map[srcIndex][destIndex].initWeight=weight;
      map[destIndex][srcIndex].initWeight=weight;
    }
    
  }
  std::unordered_map<std::string, std::pair<std::string, std::string>> Map::getRoute() {
    std::unordered_map<std::string, std::pair<std::string, std::string>> route;
    int n=map.size();
    std::vector<bool> visited(n, false);
    std::vector<int> parent(n, -1);
    std::vector<int> distance(n, INT_MAX);
    distance[0] = 0;
    // visited[0] = true;
    parent[0] = -1;
    while (true) {
      int minDistance = INT_MAX;
      int minIndex = -1;
      for (int i = 0; i < n; ++i) {
        if (!visited[i] && distance[i] < minDistance) {
          minDistance = distance[i];
          minIndex = i;
        }
      }
      if (minIndex == -1) {
        break;
      }
      visited[minIndex] = true;
      for (int i = 0; i < n; ++i) {
        if (!visited[i] && map[minIndex][i].weight > 0 && distance[minIndex] + map[minIndex][i].weight < distance[i]) {
          distance[i] = distance[minIndex] + map[minIndex][i].weight;
          parent[i] = minIndex;
        }
      }
    }
    for (int i = 1; i < n; ++i) {
      if (visited[i]) {
        int par=parent[i];
        int cur=i;
        while(par!=-1){
          if(par==0){
            route[key2node[i]]=std::make_pair(segment2interface[par][map[par][cur].segment],segment2interface[cur][map[par][cur].segment]);
          }
          cur=par;
          par=parent[par];
        }
      }
    }
    for (const auto& [key, value] : route) {
      spdlog::debug("Route: {} -> {} -> {}", key, value.first, value.second);
    }
    return route;
  }
  void Map::setMaxWeight(const std::string& host1,const std::string& host2){
    int src=node2key[host1];
    int dest=node2key[host2];
    map[src][dest].weight=INT_MAX-1;
    map[dest][src].weight=INT_MAX-1;
  }
  void Map::setInitWeight(const std::string& host1,const std::string& host2){
    int src=node2key[host1];
    int dest=node2key[host2];
    map[src][dest].weight=map[src][dest].initWeight;
    map[dest][src].weight=map[dest][src].initWeight;
  }

}