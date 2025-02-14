//
// Created by yc on 25-2-14.
//


#ifndef SUBSCRIBE_HPP
#define SUBSCRIBE_HPP
#include <memory>
#include<string>
#include"Node.hpp"

namespace Hnu::Middleware {
  //TODO: 不使用共享指针
  class Subscribe : public std::enable_shared_from_this<Subscribe> {
  private:
    std::string m_name;
    std::weak_ptr<Node> m_node;
  };

} // Middleware
// Hnu

#endif //SUBSCRIBE_HPP
