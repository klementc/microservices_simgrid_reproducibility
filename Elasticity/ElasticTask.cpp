#include <vector>
#include <memory>
#include <queue>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include "simgrid/s4u.hpp"
#include "simgrid/kernel/future.hpp"
#include "simgrid/plugins/load.h"
#include <simgrid/Exception.hpp>

#include "ElasticTask.hpp"

// using namespace simgrid;
// using namespace s4u;
namespace sg_microserv {

XBT_LOG_NEW_DEFAULT_CATEGORY(elastic, "elastic tasks");

#ifdef USE_JAEGERTRACING

std::shared_ptr<opentracing::v3::Tracer> setUpTracer(const char* configFilePath, std::string name) {
  XBT_INFO("set up jaeger tracer for %s", name.c_str());
    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    auto tracer = jaegertracing::Tracer::make(
        name, config, jaegertracing::logging::consoleLogger());
    return tracer;
}
#endif

ElasticTaskManager::ElasticTaskManager(std::string name,
  std::vector<std::string> incMailboxes, std::string jaegConfigFile)
  : serviceName_(name), incMailboxes_(incMailboxes), nextHost_(0),
    keepGoing(true), defCPUCost_(1e7), waitingReqAmount_(0),
    bootDuration_(0), executingReqAmount_(0), counterExecSlot_(0),
    parallelTasksPerInst_(100), defCPURatio_(1) {
  XBT_DEBUG("Creating TaskManager %s", serviceName_.c_str());
  sg_host_load_plugin_init();
  sleep_sem = simgrid::s4u::Semaphore::create(0);
  modif_sem_ = simgrid::s4u::Semaphore::create(1);

#ifdef USE_JAEGERTRACING
  tracer_ = setUpTracer(jaegConfigFile.c_str(), serviceName_.c_str());
#endif
}
ElasticTaskManager::ElasticTaskManager(std::string name, std::vector<std::string> incMailboxes)
  : ElasticTaskManager(name, incMailboxes, "config.yml")
{}

ElasticTaskManager::ElasticTaskManager(std::string name)
  : ElasticTaskManager(name, std::vector<std::string>(1, name), "config.yml")
{}

void ElasticTaskManager::setExecAmount(int64_t pr) {
  xbt_assert(pr >= 0, "Cannot have a negative processRatio. you asked for: %d", pr);
  XBT_DEBUG("changed default process cost per request: %d -> %d", defCPUCost_, pr);
  defCPUCost_ = pr;
}

void ElasticTaskManager::setExecRatio(double cost) {
  xbt_assert(cost >= 0, "Cannot have a negative amount of IO, you asked for: %lf", cost);
  XBT_DEBUG("changed default I/O cost per request: %lf -> %lf", defCPURatio_, cost);
  defCPURatio_ = cost;
}

void ElasticTaskManager::setParallelTasksPerInst(int s) {
  xbt_assert(s > 0, "Instances cannot execute a negative amount of tasks in parallel");
  XBT_DEBUG("new max amount of parallel tasks per instance: %d -> %d", parallelTasksPerInst_, s);
  parallelTasksPerInst_ = s;
}

void ElasticTaskManager::setExecAmountFunc(std::function<double(TaskDescription*)> costReqType) {
  XBT_DEBUG("Set a new process cost function.");
  fCPUCost = costReqType;
}

void ElasticTaskManager::setExecRatioFunction(std::function<double(TaskDescription*)> costIOType) {
  XBT_DEBUG("Set a new IO cost function.");
  fCPURatio_ = costIOType;
}


void ElasticTaskManager::addHost(simgrid::s4u::Host *host) {
  availableHostsList_.push_back(host);
  TaskInstance* ti = new TaskInstance(this, serviceName_+"_data", outputFunction, parallelTasksPerInst_);
  tiList.push_back(ti);

  XBT_DEBUG("Created a new service instance on host %s. Total # instances is now %d",
     host->get_cname(), availableHostsList_.size());
  simgrid::s4u::Actor::create("TI"+boost::uuids::to_string(boost::uuids::random_generator()()), host, [&]{ti->run();});
}


void ElasticTaskManager::setBootDuration(double bd) {
  xbt_assert(bd >= 0, "Boot time has to be non negative, you asked for %lf", bd);
  bootDuration_ = bd;
}

void ElasticTaskManager::trigger(TaskDescription* td) {
  xbt_assert(td != nullptr, "Triggered a nullptr, problem");
  // when we trigger an event, it means it reached the service correctly, thus we made a hop
  td->nbHops++;
  td->date = simgrid::s4u::Engine::get_instance()->get_clock();
  nextEvtQueue.push(td);
  // one more pending request
  modifWaitingReqAmount(1);
  sleep_sem->release();
}

simgrid::s4u::Host* ElasticTaskManager::removeHost(int i) {
  xbt_assert(availableHostsList_.size() > 1,
    "You asked for removing the last instance of the service. Cannot have 0 instances.");
  simgrid::s4u::Host* h;
  if (i < availableHostsList_.size()) {
    h =  availableHostsList_.at(i);
    availableHostsList_.erase(availableHostsList_.begin()+i);
    TaskInstance* ti = tiList.at(i);
    ti->kill();
    delete ti;
    tiList.erase(tiList.begin()+i);
  } else {
    XBT_INFO("Cannot remove element at position %d, overflow", i);
  }
  return h;
}

unsigned int ElasticTaskManager::getInstanceAmount() {
  return availableHostsList_.size();
}
std::vector<double> ElasticTaskManager::getCPULoads() {
  std::vector<double> v;
  for (int i=0; i < availableHostsList_.size() ; i++) {
    v.push_back(sg_host_get_avg_load(availableHostsList_.at(i)));
    sg_host_load_reset(availableHostsList_.at(i));
  }
  return v;
}

void ElasticTaskManager::modifExecutingReqAmount(int n) {
  modif_sem_->acquire();
  executingReqAmount_+=n;
  xbt_assert(executingReqAmount_ >= 0,
    "cannot have less than 0 executing requests");
  modif_sem_->release();
}

void ElasticTaskManager::modifWaitingReqAmount(int n) {
  modif_sem_->acquire();
  waitingReqAmount_+=n;
  xbt_assert(waitingReqAmount_ >= 0, "cannot have less than 0 waiting requests");
  modif_sem_->release();
}

int64_t ElasticTaskManager::getAmountOfWaitingRequests() {
  return waitingReqAmount_;
}

int64_t ElasticTaskManager::getAmountOfExecutingRequests() {
  return executingReqAmount_;
}

void ElasticTaskManager::setOutputFunction(std::function<void(TaskDescription*)> f) {
  outputFunction = f;
}

#ifdef USE_JAEGERTRACING
std::shared_ptr<opentracing::v3::Tracer> ElasticTaskManager::getTracer() {
  return tracer_;
}
#endif

double ElasticTaskManager::reqPerSec() {
  xbt_assert(!fCPUCost, "TODO: cannot compute the reqpersec value with this function if cost function is set");
  double tot = 0;
  for (auto i : tiList) {
    simgrid::s4u::Host* a = i->getRunningHost();
    if (a)
      tot+=(a->get_pstate_speed(a->get_pstate())*a->get_core_count())/defCPUCost_;
  }
  return tot;
}

void ElasticTaskManager::setReqNames(std::function<std::string(RequestType)> rnToStr) {
  reqTypeToStr_ = rnToStr;
}

double ElasticTaskManager::getCPUExecAmount(TaskDescription* e) {
  // if the function is defined, use it, otherwise use the single value
  if (fCPUCost) {
    return fCPUCost(e);
  } else {
    return defCPUCost_;
  }
}

double ElasticTaskManager::getExecRatio(TaskDescription* e) {
  double ratio = -1;
  if (fCPURatio_) {
    ratio = fCPURatio_(e);
  } else {
    ratio = defCPURatio_;
  }
  xbt_assert(ratio >= 0, "Cannot use an execution ratio below 0, you provided %lf", ratio);
  XBT_DEBUG("Execution ratio for request %p = %lf", e, ratio);
  return ratio;
}

std::string ElasticTaskManager::getStrFromReqType(RequestType r) {
  if (reqTypeToStr_) {
    return reqTypeToStr_(r);
  } else {
    XBT_DEBUG("No reqTypeToStr_ provided, return service name");
    return serviceName_;
  }
}

/**
 * Will stop run()'s infinite loop
 */
void ElasticTaskManager::kill() {
  XBT_INFO("KILL ETM");
  keepGoing = false;
  for (int i=0 ; i < tiList.size() ; i++) {
    tiList.at(i)->kill();
  }

  for (auto a : pollers_) {
    a->kill();
  }
}

void ElasticTaskManager::pollnet(std::string mboxName) {
  simgrid::s4u::Mailbox* recvMB = simgrid::s4u::Mailbox::by_name(mboxName.c_str());

  int parComSize = parallelTasksPerInst_;
  XBT_DEBUG("start poll with %d parallel get_async", parallelTasksPerInst_);
  TaskDescription* tasksV[parComSize];
  std::vector<simgrid::s4u::CommPtr> commV;
  for (int i=0 ; i < parComSize ; i++) {
    commV.push_back(recvMB->get_async<TaskDescription>(&(tasksV[i])));
  }

  while (keepGoing) {
    int newMsgPos = simgrid::s4u::Comm::wait_any(&commV);
    TaskDescription* taskRequest = tasksV[newMsgPos];

    // set amount of computation for the instance
    taskRequest->queueArrival = simgrid::s4u::Engine::get_clock();

    commV.erase(commV.begin() + newMsgPos);
    commV.insert(commV.begin() + newMsgPos, recvMB->get_async<TaskDescription>(&(tasksV[newMsgPos])));

    XBT_DEBUG("Received %p index: %d", taskRequest, newMsgPos);

    if (incMailboxes_.size() == 1) {
      trigger(taskRequest);
      sleep_sem->release();
    } else {
      std::vector<TaskDescription*> v =
        (tempData.find(taskRequest->id_) != tempData.end()) ?
          tempData.find(taskRequest->id_)->second : std::vector<TaskDescription*>();

      v.push_back(taskRequest);
      tempData.insert(std::pair<boost::uuids::uuid, std::vector<TaskDescription*>>(taskRequest->id_, v));

      if (v.size() == incMailboxes_.size()) {
        XBT_DEBUG("Received %d for %p, trigger now", v.size(), taskRequest);
        trigger(taskRequest);
        // remove data
        tempData.erase(taskRequest->id_);
        sleep_sem->release();
      }
    }
  }
}

/**
 * Supervisor of the elastictasks
 * Fetches events at their execution time and creates a new microtask
 * on one of the provided hosts
 */
void ElasticTaskManager::run() {
  int64_t task_count = 0;

  for (auto s : incMailboxes_) {
    simgrid::s4u::Mailbox* rec = simgrid::s4u::Mailbox::by_name(s.c_str());
    pollers_.push_back(simgrid::s4u::Actor::create(
      serviceName_+"_"+s+"polling", simgrid::s4u::Host::current(), [&]{pollnet(s);}));
    XBT_INFO("polling on mailbox %s", s.c_str());
  }

  while (1) {
    for (std::vector<simgrid::s4u::CommPtr>::iterator it = pending_comms.begin(); it != pending_comms.end();) {
      if ((*it)->test())
        it = pending_comms.erase(it);
      else
        it++;
    }
    // execute events that need be executed now
    while (!nextEvtQueue.empty() && nextEvtQueue.top()->date <= simgrid::s4u::Engine::get_instance()->get_clock()) {
      EvntQ *currentEvent = nextEvtQueue.top();
      nextEvtQueue.pop();
      XBT_DEBUG("add: %p", currentEvent);
      TaskDescription* t = dynamic_cast<TaskDescription*>(currentEvent);
      if (availableHostsList_.size() <= nextHost_)
        nextHost_ = 0;
      simgrid::s4u::Mailbox* mbp = simgrid::s4u::Mailbox::by_name(serviceName_+"_data");

      /* Async version TODO: clean pending vector */
      XBT_DEBUG("Send %p to task", t);
      t->flopsPerServ.push_back(getCPUExecAmount(t));
      if (!t->dSize || t->dSize == -1) {t->dSize = 1; XBT_DEBUG("NO SIZE SET, PUT TO 1");}
      simgrid::s4u::CommPtr comm = mbp->put_async(t, (t->dSize == -1) ? 1 : t->dSize);
      pending_comms.push_back(comm);

      ++task_count;

      nextHost_++;
      nextHost_ = nextHost_ % availableHostsList_.size();
      }
    if (!keepGoing) {
      break;
    }

    XBT_DEBUG("RUN: going to sleep on semaphore");
    if (!nextEvtQueue.empty()) {
      sleep_sem->acquire_timeout(nextEvtQueue.top()->date - simgrid::s4u::Engine::get_instance()->get_clock());
    } else {
      sleep_sem->acquire_timeout(999.0);
    }
  }
}

ElasticTaskManager::~ElasticTaskManager() {
  // cancel active communications
  for (auto c : pending_comms) {
    c->cancel();
  }

  // delete instances
  tiList.clear();
}

}  // namespace sg_microserv
