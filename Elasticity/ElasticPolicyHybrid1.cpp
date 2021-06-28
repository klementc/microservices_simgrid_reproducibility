#include <vector>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <simgrid/s4u/Actor.hpp>
#include "simgrid/s4u/Engine.hpp"

#include "ElasticPolicy.hpp"
#include "ElasticTask.hpp"

// using namespace simgrid;
// using namespace s4u;
namespace sg_microserv {

/**
 * Policy trying to reproduce Algorithm1 of the following paper
 * https://dl.acm.org/doi/pdf/10.1145/2287036.2287044?casa_token=ZavTCgDn1dcAAAAA:LA2SJsvCSk1CKFUT0Y_6r5Ho6VaoI0FOhMvh0lQpnRXVaFpsafjh7XntWafzh4wjwvFm591K68l0Ww
 *
 * It consists of a hybrid autoscaler
 */
XBT_LOG_NEW_DEFAULT_CATEGORY(elasticPolicyProactive1, "Elastic tasks policy proactive");


ElasticPolicyHybrid1::ElasticPolicyHybrid1(double interval, double k, double r, double mAvg)
  : ElasticPolicy(interval), k_(k), r_(r), mAvg_(mAvg) {}

void ElasticPolicyHybrid1::run() {
  int instanceToStartIndex = 0;

  XBT_INFO("ElasticPolicyreactive1 activated");
  // make your decision (here only one group should be in the taskgroup)
  xbt_assert(getTasks().size() == 1, "Reactive1 policy operates on single groups");


  double proactive_aggregator = 0;
  // time between estimations
  double Td = 1;
  // current amount of slots since last estimation
  double Tc = 0;

  std::vector<double> Lvec = std::vector<double>();
  std::vector<double> Lvectot = std::vector<double>();
  double Ltd;
  double Lt;
  double P;
  double Dtild;

  ElasticTaskManager* etm = getTasks().at(0);
  while (isActive()) {
    // wait until next scaling
    simgrid::s4u::this_actor::sleep_for(getUpdateInterval());

    // one new slot
    Tc++;

    // Update R(t), A(t), B(t), and E(t) from monitoring data
    double B = etm->getAmountOfWaitingRequests();
    double E = etm->getAmountOfExecutingRequests();

    double L = E + (B/r_);
    // used for metrics computation
    Lvec.push_back(L);
    Lvectot.push_back(L);

    double R = etm->getInstanceAmount()*mAvg_*getUpdateInterval();

    double D = L - R;
    XBT_INFO("L - R = %lf - %lf = %lf", L, R, D);

    if (Tc >= Td) {
      Tc = 0;

      // Ltd (eq. 5)
      Ltd = std::accumulate(Lvec.begin(), Lvec.end(), 0.0) / Lvec.size();
      // Lt (eq. 6)
      Lt = std::accumulate(Lvectot.begin(), Lvectot.end(), 0.0) / (Lvectot.size());
      // P (eq. 7)
      P = D/Lt;
      // Dtild (eq. 8)
      Dtild = (Ltd/Lt)*D;
      XBT_INFO("fff %lf %lf %lf %lf %lf", Ltd, Lt, std::accumulate(Lvec.begin(), Lvec.end(), 0.0), Dtild, D);
      // Td
      Td = (k_== 0 || Dtild == 0) ? 1 : k_/Dtild;

      // clear
      Lvec.clear();
    }

    int Nreactive = std::ceil(D/(mAvg_*getUpdateInterval()));
    proactive_aggregator += Dtild/(mAvg_*getUpdateInterval());
    int nProactive = std::floor(proactive_aggregator);
    proactive_aggregator -= nProactive;


    int execInSlot = etm->getCounterExecSlot();
    std::vector<double> lv = etm->getCPULoads();
    double avgLoad = std::accumulate(lv.begin(), lv.end(), 0.0) / lv.size();
    std::string s = "";
    XBT_INFO("%f %d %d %d %d %f stats", avgLoad, etm->getInstanceAmount(), etm->getAmountOfWaitingRequests(),
      etm->getAmountOfExecutingRequests(), execInSlot, etm->reqPerSec()*getUpdateInterval());

    etm->resetCounterExecSlot();

    if (Nreactive > k_) {
      if (nProactive > 0) {
        // deploy Np + Nr
        deploy(nProactive+Nreactive, etm);
      } else {
        // deploy Nr
        deploy(Nreactive, etm);
      }
    } else {
      // (un)deploy
      deploy(static_cast<int>(nProactive), etm);
    }

    XBT_INFO("%f %d %d %d %d %f stats", avgLoad, etm->getInstanceAmount(), etm->getAmountOfWaitingRequests(),
      etm->getAmountOfExecutingRequests(), execInSlot, etm->reqPerSec()*getUpdateInterval());
  }
}

void ElasticPolicyHybrid1::deploy(int ninst, ElasticTaskManager* etm) {
  XBT_INFO("%d", ninst);
  if (ninst > 0) {
    for (int i=0 ; i < ninst ; i++) {
      if (hostPool_.size() > 0) {
        etm->addHost(hostPool_.at(0));
        hostPool_.erase(hostPool_.begin());
      }
    }
  } else if (ninst < 0) {
    for (int i=0 ; i < -ninst ; i++) {
      // if more than one instance, remove one
      if (etm->getInstanceAmount() > 1) {
        simgrid::s4u::Host* h = etm->removeHost(0);
        if (h)
          hostPool_.push_back(h);
      }
    }
  }
}

}  // namespace sg_microserv
