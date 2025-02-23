//
// Created by yc on 25-2-22.
//

#ifndef PUBLISHINTERFACE_HPP
#define PUBLISHINTERFACE_HPP
#include <memory>

namespace Hnu::Middleware {
  class PublishInterface :public std::enable_shared_from_this<PublishInterface> {
  public:
    virtual ~PublishInterface()=default;
  };
}


#endif //PUBLISHINTERFACE_HPP
