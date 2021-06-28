#ifndef ELASTICITY_ELASTICPOLICY_HPP_
#define ELASTICITY_ELASTICPOLICY_HPP_

#include <vector>

#include "ElasticTask.hpp"


// namespace simgrid {
// namespace s4u {
namespace sg_microserv {

class ElasticPolicy {
 protected:
    std::vector<ElasticTaskManager*> etmList_;
    std::vector<simgrid::s4u::Host*> hostPool_;
    bool keepRunning_;
    double updateInterval_;
 public:
    explicit ElasticPolicy(double interval)
      : updateInterval_(interval), keepRunning_(true) {}

    inline void addElasticTaskManager(ElasticTaskManager* etm) {
      etmList_.push_back(etm);
    }

    inline void addHost(simgrid::s4u::Host* host) {
      hostPool_.push_back(host);
    }
    void kill() {keepRunning_ = false;}
    inline bool isActive() {return keepRunning_;}
    inline double getUpdateInterval() {return updateInterval_;}
    inline std::vector<ElasticTaskManager*> getTasks() {return etmList_;}
    virtual void run() = 0;
};

/**
 * Dumb threshold based policy
 */
class ElasticPolicyCPUThreshold: public ElasticPolicy {
 private:
    double upperCPUThresh_;
    double lowCPUThresh_;

 public:
    ElasticPolicyCPUThreshold(double inter, double lowCPUT, double highCPUT);
    virtual void run();
};

/**
 * Reactive only policy taken from
 * "Efficient Provisioning of Bursty Scientific Workloads on the Cloud Using Adaptive
 * Elasticity Control"
 */
class ElasticPolicyReactive1: public ElasticPolicy {
 private:
    double k_;
    double r_;
    double mAvg_;

 public:
    ElasticPolicyReactive1(double interval, double k, double r, double mAvg);
    virtual void run();
};

/**
 * Hybrid policy taken from
 * "Efficient Provisioning of Bursty Scientific Workloads on the Cloud Using Adaptive
 * Elasticity Control"
 */
class ElasticPolicyHybrid1: public ElasticPolicy {
 private:
    double k_;
    double r_;
    double mAvg_;

 public:
    ElasticPolicyHybrid1(double interval, double k, double r, double mAvg);
    virtual void run();
    void deploy(int nInst, ElasticTaskManager* etm);
};

}  // namespace sg_microserv

#endif  // ELASTICITY_ELASTICPOLICY_HPP_
