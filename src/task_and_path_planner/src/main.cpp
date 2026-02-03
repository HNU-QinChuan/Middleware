//
// Created by TaskPlanner on Fix.
//

#include "PathCalculator.hpp"
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    // Set log level
    spdlog::set_level(spdlog::level::debug);
    
    // Create PathCalculator instance
    auto pathCalculator = std::make_shared<TaskPlanner::PathCalculator>("PathCalculatorNode");
    
    spdlog::info("Initializing PathCalculator...");
    
    // Initialize - this will now properly load the domain
    pathCalculator->Init();
    
    spdlog::info("Testing path planning...");
    
    // Test path planning - this will now work with loaded domain
    std::string plan = pathCalculator->GetPlanPath();
    
    if (!plan.empty()) {
        spdlog::info("Successfully generated path plan!");
    } else {
        spdlog::error("Failed to generate path plan");
    }
    
    return 0;
}