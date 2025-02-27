//
// Created by yc on 25-2-22.
//

#pragma once

#include <memory>

namespace Hnu::Middleware {
  class PublishInterface :public std::enable_shared_from_this<PublishInterface> {
  public:
    virtual ~PublishInterface()=default;
  };
}



