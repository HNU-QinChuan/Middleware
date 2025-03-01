//
// Created by yc on 25-2-22.
//

#pragma once

#include <memory>

namespace Hnu::Middleware {
  class PublisherInterface :public std::enable_shared_from_this<PublisherInterface> {
  public:
    virtual ~PublisherInterface()=default;
  };
}



