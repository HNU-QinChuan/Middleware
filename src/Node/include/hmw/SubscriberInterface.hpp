//
// Created by yc on 25-2-23.
//

#pragma once

#include <memory>

namespace Hnu::Middleware {
  class SubscriberInterface :public std::enable_shared_from_this<SubscriberInterface> {
  public:
    virtual ~SubscriberInterface()=default;
  };
}

