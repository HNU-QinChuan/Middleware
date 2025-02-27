//
// Created by yc on 25-2-23.
//

#pragma once

#include <memory>

namespace Hnu::Middleware {
  class SubscribeInterface :public std::enable_shared_from_this<SubscribeInterface> {
  public:
    virtual ~SubscribeInterface()=default;
  };
}

