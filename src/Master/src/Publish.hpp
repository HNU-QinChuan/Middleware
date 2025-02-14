//
// Created by yc on 25-2-14.
//


#ifndef PUBLISH_HPP
#define PUBLISH_HPP

#include <memory>
#include<string>
#include"Node.hpp"

namespace Hnu::Middleware {
  //TODO: 不使用共享指针
  class Publish:public std::enable_shared_from_this<Publish> {

  private:
    std::string m_name;
    std::weak_ptr<Node> m_node;
  };

}

#endif //PUBLISH_HPP
