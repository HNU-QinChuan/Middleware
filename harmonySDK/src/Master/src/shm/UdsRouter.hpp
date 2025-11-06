//
// Created by yc on 25-2-28.
//


#pragma once
#include<unordered_map>
#include<string>
#include<boost/beast/http.hpp>
#include <spdlog/spdlog.h>


namespace Hnu::Middleware {
  namespace http=boost::beast::http;
  using Request=http::request<http::string_body>;
  using Response=http::response<http::string_body>;
  class UdsRouter {
  public:
    UdsRouter(const UdsRouter&)=delete;
    UdsRouter& operator=(const UdsRouter&)=delete;
    UdsRouter(const UdsRouter&&)=delete;
    UdsRouter& operator=(const UdsRouter&&)=delete;
    static void registerController(const std::string& path,http::verb verb,std::function<void(Request&,Response&)> func);
    static void handle(Request& req,Response& res);
  private:
    UdsRouter()=default;
    static UdsRouter m_instance;
    static UdsRouter& getInstance();
    std::unordered_map<std::string,std::unordered_map<http::verb,std::function<void(Request&,Response&)>>> m_router;
  };

  template<typename T>
  class Controllerregister {
  public:
    template<typename... Args>
    Controllerregister(const char* path, Args&&... args): path(path),controller(new T) {
      doRegister(args...);
    }
    template<typename... Args>
    void doRegister(http::verb verb,void (T::*funcPtr)(Request&,Response&),Args&&... args){
      UdsRouter::registerController(path,verb,std::bind(funcPtr,controller,std::placeholders::_1,std::placeholders::_2));
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




} // Middleware
// Hnu


