//
// Created by TaskPlanner on Fix.
//

#pragma once

#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <hmw/Subscriber.hpp>
#include <TaskPlanner/GlobalPathPlan.pb.h>
#include <TaskPlanner/GlobalPathPlanFeedback.pb.h>
#include <memory>
#include <string>
#include <chrono>

// Forward declarations for PlanSys2 - these would normally come from plansys2 headers
namespace plansys2 {
    class DomainExpertClient {
    public:
        DomainExpertClient() = default;
        virtual ~DomainExpertClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { return true; }
        virtual bool addDomain(const std::string& domain_str) { return true; }
        virtual std::string getDomain() { return ""; }
    };
    
    class PlannerClient {
    public:
        PlannerClient() = default;
        virtual ~PlannerClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { return true; }
        virtual std::string getPlan(const std::string& domain, const std::string& problem) { return ""; }
    };
    
    class ProblemExpertClient {
    public:
        ProblemExpertClient() = default;
        virtual ~ProblemExpertClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { return true; }
    };
    
    class ExecutorClient {
    public:
        ExecutorClient() = default;
        virtual ~ExecutorClient() = default;
        virtual bool wait_for_service(std::chrono::duration<int> timeout) { return true; }
    };
}

namespace TaskPlanner
{
    class PathCalculator : public Hnu::Middleware::Node
    {
    public:
        explicit PathCalculator(const std::string &name);
        virtual ~PathCalculator();
        
        // Main initialization function
        void Init();
        
        // Path planning function
        std::string GetPlanPath();
        
    protected:
        // PlanSys2 clients
        std::shared_ptr<plansys2::DomainExpertClient> _domainExpert;
        std::shared_ptr<plansys2::PlannerClient> _plannerClient;
        std::shared_ptr<plansys2::ProblemExpertClient> _problemExpert;
        std::shared_ptr<plansys2::ExecutorClient> _executorClient;
        
        // Domain loading status
        bool domain_loaded_;
    };
}