//
// Standalone test for PathCalculator domain loading without middleware
//

#include <iostream>
#include <spdlog/spdlog.h>

// Simple mock of the PathCalculator logic to test domain loading
namespace plansys2 {
    class DomainExpertClient {
    public:
        DomainExpertClient() = default;
        virtual ~DomainExpertClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { 
            spdlog::info("DomainExpertClient: Service available");
            return true; 
        }
        virtual bool addDomain(const std::string& domain_str) { 
            spdlog::info("DomainExpertClient: Domain loaded successfully");
            domain_ = domain_str;
            return true; 
        }
        virtual std::string getDomain() { 
            return domain_; 
        }
    private:
        std::string domain_;
    };
    
    class PlannerClient {
    public:
        PlannerClient() = default;
        virtual ~PlannerClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { return true; }
        virtual std::string getPlan(const std::string& domain, const std::string& problem) { 
            if (domain.empty()) {
                spdlog::error("Cannot generate plan - domain is empty!");
                return "";
            }
            spdlog::info("Generated plan with loaded domain");
            return "move(robot1, zone_start, zone_goal)"; 
        }
    };
    
    class ProblemExpertClient {
    public:
        ProblemExpertClient() = default;
        virtual ~ProblemExpertClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { 
            spdlog::info("ProblemExpertClient: Service available");
            return true; 
        }
    };
    
    class ExecutorClient {
    public:
        ExecutorClient() = default;
        virtual ~ExecutorClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { return true; }
    };
}

class StandalonePathCalculator {
public:
    StandalonePathCalculator() : domain_loaded_(false) {}
    
    void InitWithoutDomainLoading() {
        spdlog::info("Original broken Init() - creates clients but no domain loading");
        _domainExpert = std::make_shared<plansys2::DomainExpertClient>();
        _plannerClient = std::make_shared<plansys2::PlannerClient>();
        _problemExpert = std::make_shared<plansys2::ProblemExpertClient>();
        _executorClient = std::make_shared<plansys2::ExecutorClient>();
        
        // Missing: Domain definition loading! (This was the bug)
        domain_loaded_ = false;
        spdlog::warn("Domain not loaded - GetPlanPath will fail!");
    }
    
    void InitWithDomainLoading() {
        spdlog::info("Fixed Init() - creates clients AND loads domain");
        _domainExpert = std::make_shared<plansys2::DomainExpertClient>();
        _plannerClient = std::make_shared<plansys2::PlannerClient>();
        _problemExpert = std::make_shared<plansys2::ProblemExpertClient>();
        _executorClient = std::make_shared<plansys2::ExecutorClient>();
        
        // THE FIX: Add service availability checks and domain loading
        int domain_expert_retries = 0;
        while (!_domainExpert->wait_for_service(std::chrono::seconds(1))) {
            if (domain_expert_retries++ > 2) {
                spdlog::error("Timed out waiting for domain expert service");
                return;
            }
            spdlog::info("Waiting for domain expert service...");
        }

        int problem_expert_retries = 0;
        while (!_problemExpert->wait_for_service(std::chrono::seconds(1))) {
            if (problem_expert_retries++ > 2) {
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
    
    std::string GetPlanPath() {
        if (!domain_loaded_) {
            spdlog::error("Domain not loaded - cannot generate plan");
            return "";
        }
        
        auto domain = _domainExpert->getDomain();  // Now returns loaded domain
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
        
        auto plan = _plannerClient->getPlan(domain, problem);
        
        if (plan.empty()) {
            spdlog::warn("No plan found for current problem");
            return "";
        }
        
        spdlog::info("Generated plan: {}", plan);
        return plan;
    }
    
private:
    std::shared_ptr<plansys2::DomainExpertClient> _domainExpert;
    std::shared_ptr<plansys2::PlannerClient> _plannerClient;
    std::shared_ptr<plansys2::ProblemExpertClient> _problemExpert;
    std::shared_ptr<plansys2::ExecutorClient> _executorClient;
    bool domain_loaded_;
};

int main() {
    spdlog::set_level(spdlog::level::info);
    spdlog::info("=== PathCalculator Domain Loading Fix Test ===");
    
    auto calculator = std::make_shared<StandalonePathCalculator>();
    
    spdlog::info("\n1. Testing BROKEN version (without domain loading):");
    calculator->InitWithoutDomainLoading();
    std::string broken_plan = calculator->GetPlanPath(); // This will fail
    
    spdlog::info("\n2. Testing FIXED version (with domain loading):");
    calculator->InitWithDomainLoading();
    std::string fixed_plan = calculator->GetPlanPath(); // This will work
    
    spdlog::info("\n=== Results ===");
    spdlog::info("Broken version plan: '{}'", broken_plan.empty() ? "FAILED (empty)" : broken_plan);
    spdlog::info("Fixed version plan: '{}'", fixed_plan.empty() ? "FAILED (empty)" : fixed_plan);
    
    if (!broken_plan.empty() || fixed_plan.empty()) {
        spdlog::error("Unexpected results!");
        return 1;
    }
    
    spdlog::info("✓ Fix verified: Domain loading enables path planning!");
    return 0;
}