//
// Created by TaskPlanner on Fix.
//

#include "PathCalculator.hpp"
#include <spdlog/spdlog.h>

namespace TaskPlanner
{
    PathCalculator::PathCalculator(const std::string &name) : Node(name), domain_loaded_(false)
    {
        spdlog::debug("PathCalculator constructor");
    }

    PathCalculator::~PathCalculator()
    {
        spdlog::debug("PathCalculator destructor");
    }

    void PathCalculator::Init()
    {
        // Original problematic code - creates clients but doesn't load domain
        _domainExpert = std::make_shared<plansys2::DomainExpertClient>();
        _plannerClient = std::make_shared<plansys2::PlannerClient>();
        _problemExpert = std::make_shared<plansys2::ProblemExpertClient>();
        _executorClient = std::make_shared<plansys2::ExecutorClient>();
        
        // THE FIX: Add service availability checks and domain loading
        
        // Wait for PlanSys2 services to be available
        int domain_expert_retries = 0;
        while (!_domainExpert->wait_for_service(std::chrono::seconds(1))) {
            if (domain_expert_retries++ > 10) {
                spdlog::error("Timed out waiting for domain expert service");
                return;
            }
            spdlog::info("Waiting for domain expert service...");
        }

        int problem_expert_retries = 0;
        while (!_problemExpert->wait_for_service(std::chrono::seconds(1))) {
            if (problem_expert_retries++ > 10) {
                spdlog::error("Timed out waiting for problem expert service");
                return;
            }
            spdlog::info("Waiting for problem expert service...");
        }

        // Load PDDL domain definition
        std::string domain_str = R"(
            (define (domain robot_navigation)
              (:requirements :strips :typing)
              
              (:types 
                robot zone
              )
              
              (:predicates 
                (robot_at ?r - robot ?z - zone)
                (can-move ?from - zone ?to - zone)
              )
              
              (:action move
                :parameters (?r - robot ?from - zone ?to - zone)
                :precondition (and 
                  (robot_at ?r ?from) 
                  (can-move ?from ?to)
                )
                :effect (and 
                  (not (robot_at ?r ?from)) 
                  (robot_at ?r ?to)
                )
              )
            )
        )";

        if (!_domainExpert->addDomain(domain_str)) {
            spdlog::error("Failed to add domain definition");
            return;
        }

        spdlog::info("Domain definition loaded successfully!");
        domain_loaded_ = true;
    }

    std::string PathCalculator::GetPlanPath()
    {
        if (!domain_loaded_) {
            spdlog::error("Domain not loaded - cannot generate plan");
            return "";
        }
        
        // This was failing before because domain was empty
        auto domain = _domainExpert->getDomain();  // Now returns loaded domain
        
        // Example problem - in real implementation this would come from current state
        std::string problem = R"(
            (define (problem robot_nav_problem)
              (:domain robot_navigation)
              (:objects 
                robot1 - robot
                zone_start zone_goal - zone
              )
              (:init 
                (robot_at robot1 zone_start)
                (can-move zone_start zone_goal)
              )
              (:goal 
                (robot_at robot1 zone_goal)
              )
            )
        )";
        
        auto plan = _plannerClient->getPlan(domain, problem);  // Now works with loaded domain
        
        if (plan.empty()) {
            spdlog::warn("No plan found for current problem");
            return "";
        }
        
        spdlog::info("Generated plan: {}", plan);
        return plan;
    }
}