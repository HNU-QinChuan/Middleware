//
// Created by yc on 25-2-15.
//


#ifndef NODE_HPP
#define NODE_HPP
#include <memory>


namespace Hnu::Middleware {

  class Node : public std::enable_shared_from_this<Node> {
  public:
    Node(const std::string &name);
  private:
    std::string name;
  };

} // Middleware
// Hnu

#endif //NODE_HPP
