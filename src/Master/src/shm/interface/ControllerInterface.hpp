//
// Created by yc on 25-2-28.
//

#pragma once

#include<boost/beast/http.hpp>


namespace Hnu::Middleware {
  namespace http=boost::beast::http;
  class ControllerInterface {
  public:
    using Request=http::request<http::string_body>;
    using Response=http::response<http::string_body>;
    virtual void handle(Request& req,Response& res)=0;

  };
} // Middleware
// Hnu
