#pragma once


#include<unordered_map>
#include<string>
#include<boost/beast/http.hpp>
#include <spdlog/spdlog.h>


namespace Hnu::Interface {
  namespace http=boost::beast::http;
  using Request=http::request<http::string_body>;
  class InterfaceRouter {
  public:
    InterfaceRouter(const InterfaceRouter&)=delete;
    InterfaceRouter& operator=(const InterfaceRouter&)=delete;
    InterfaceRouter(const InterfaceRouter&&)=delete;
    InterfaceRouter& operator=(const InterfaceRouter&&)=delete;
    static void registerController(const std::string& path,http::verb verb,std::function<void(Request&)> func);
    static void handle(Request& req);
  private:
    InterfaceRouter()=default;
    static InterfaceRouter m_instance;
    static InterfaceRouter& getInstance();
    std::unordered_map<std::string,std::unordered_map<http::verb,std::function<void(Request&)>>> m_router;
  };

  template<typename T>
  class Controllerregister {
  public:
    template<typename... Args>
    Controllerregister(const char* path, Args&&... args): path(path),controller(new T) {
      doRegister(args...);
    }
    template<typename... Args>
    void doRegister(http::verb verb,void (T::*funcPtr)(Request&),Args&&... args){
      InterfaceRouter::registerController(path,verb,std::bind(funcPtr,controller,std::placeholders::_1));
      doRegister(args...);
    }
    void doRegister(){}
    ~Controllerregister(){
      delete controller;
    }
  
    T* controller;
    std::string path;
  };

  #define CONTROLLER_REGISTER(controller_class,path, ...) \
  static Controllerregister<controller_class> registrar##controller_class( path, __VA_ARGS__);




}