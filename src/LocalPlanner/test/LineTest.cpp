#include "base/ApeLine.hpp"
#include <iostream>

int main()
{
  Eigen::Vector2d A(0.0, 0.0);
  Eigen::Vector2d B(0.0, 4.0);
  Eigen::Vector2d s(3.0, 4.0);
  DWA::apeLine line(A, B);

  std::cout << "Distance from point to line: " << line.DistanceOfPointToLine(s) << std::endl;

}