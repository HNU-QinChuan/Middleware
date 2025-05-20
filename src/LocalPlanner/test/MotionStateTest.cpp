#include "base/MotionState.hpp"

int main()
{
    DWA::State state1(0.0,0.0,0.0,1.0,1.0);
    for(int i = 0; i < 10; ++i)
    {
        state1.state_motion(1.0, 1.0, 0.1);
        std::cout << "State1: " << state1.x << ", " << state1.y << ", " << state1.yaw << ", " << state1.velocity << ", " << state1.yawrate << std::endl;
    }
}