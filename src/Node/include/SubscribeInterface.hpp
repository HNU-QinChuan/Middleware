//
// Created by yc on 25-2-23.
//

#ifndef SUBSCRIBEINTERFACE_HPP
#define SUBSCRIBEINTERFACE_HPP
#include <memory>

namespace Hnu::Middleware {
  class SubscribeInterface :public std::enable_shared_from_this<SubscribeInterface> {
  public:
    virtual ~SubscribeInterface()=default;
  };
}
#endif //SUBSCRIBEINTERFACE_HPP
