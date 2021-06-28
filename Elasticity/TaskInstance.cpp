#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

#include <simgrid/s4u.hpp>
#include <simgrid/s4u/Disk.hpp>
#include <simgrid/s4u/Io.hpp>

#include "ElasticTask.hpp"

// using namespace simgrid;
// using namespace s4u;

XBT_LOG_NEW_DEFAULT_CATEGORY(elasticInstance, "elastic task instance");

namespace sg_microserv {


TaskInstance::TaskInstance(ElasticTaskManager* etm,  std::string mbName,
  std::function<void(TaskDescription*)> outputFunction, int maxReqInInst, double bootTime)
  : etm_(etm), mbName_(mbName), outputFunction_(outputFunction),
    keepGoing_(true), maxReqInInst_(maxReqInInst), bootTime_(bootTime),
    host_(nullptr) {
  n_empty_ = simgrid::s4u::Semaphore::create(maxReqInInst);
  n_full_ = simgrid::s4u::Semaphore::create(0);

  // atm no execs and IO so block those
  sem_pollExec_ = simgrid::s4u::Semaphore::create(maxReqInInst_);
  for (int i=0 ; i < maxReqInInst_ ; i++) {sem_pollExec_->acquire();}

  /** if no disks are available on the host, add one in case the user wants to perform some I/O operations
   * IMPORTANT: replaced by an exec bound for now
   * sem_pollIO_ = s4u::Semaphore::create(maxReqInInst_);
   * for(int i=0;i<maxReqInInst_;i++){sem_pollIO_->acquire();}
   * if(simgrid::s4u::Host::current()->get_disks().size()){
   * disk_ = simgrid::s4u::Host::current()->get_disks().front();
   * XBT_DEBUG("There is already a disk ('%s') on host '%s' with bandidths R: %lf B/s, W: %lf B/s, for I/Os",
   *   disk_->get_cname() , simgrid::s4u::Host::current()->get_cname(), disk_->get_read_bandwidth(), disk_->get_write_bandwidth());
   * }else {
   *
   *  std::string diskName = "microservAutoDisk";
   * XBT_DEBUG("No disk found on host '%s', creating one with name %s, bandwidths R: %lf B/s, W: %lf B/s for I/Os",
   *  simgrid::s4u::Host::current()->get_cname(), diskName.c_str(), diskReadBW, diskWBW);
   *   disk_ = simgrid::s4u::Host::current()->create_disk(diskName, 9.6e7, 6.4e7);
   * }
   */
}

TaskInstance::TaskInstance(ElasticTaskManager* etm, std::string mbName,
      std::function<void(TaskDescription*)> outputFunction, int par)
  :TaskInstance(etm, mbName, outputFunction, par, 0)
  {}

void TaskInstance::pollTasks() {
  simgrid::s4u::Mailbox* mbp = simgrid::s4u::Mailbox::by_name(mbName_);


  // create and init parallel reception of tasks
  int parComSize =  maxReqInInst_;
  XBT_DEBUG("start poll with %d parallel get_async", maxReqInInst_);
  TaskDescription* tasksV[parComSize];

  for (int i=0 ; i < parComSize ; i++) {
    commV.push_back(mbp->get_async<TaskDescription>(&(tasksV[i])));
  }

  while (keepGoing_) {
    int newMsgPos = simgrid::s4u::Comm::wait_any(&commV);
    TaskDescription* taskRequest = tasksV[newMsgPos];
    taskRequest->instArrival = simgrid::s4u::Engine::get_clock();
    commV.erase(commV.begin()+newMsgPos);
    commV.insert(commV.begin() + newMsgPos, mbp->get_async<TaskDescription>(&(tasksV[newMsgPos])));
    // we wait until there is a slot to execute a task before putting the request in the slot
    int n = n_empty_->acquire_timeout(1000);
    if (!(n == 0))
      continue;
    reqs.push_back(taskRequest);
    XBT_DEBUG("instance received a request %p at index %d, queue size: %d", taskRequest, newMsgPos, reqs.size());

    n_full_->release();
  }
}

void TaskInstance::pollEndOfTaskExec() {
  while (keepGoing_) {
    sem_pollExec_->acquire();

    int index = simgrid::s4u::Exec::wait_any(&pending_execs);
    // finished one exec, call output function and allow for a new execution
    TaskDescription* td = execMap_.find(pending_execs.at(index))->second;
    td->endExec = simgrid::s4u::Engine::get_clock();
    etm_->modifExecutingReqAmount(-1);
    etm_->setCounterExecSlot(etm_->getCounterExecSlot()+1);

#ifdef USE_JAEGERTRACING
        auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(
            static_cast<int>(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
        // Check if no problem on spans
        if (td->parentSpans.size() > 0)
          td->parentSpans.at(td->parentSpans.size()-1)->get()->Log(
            {{"endExec", simgrid::s4u::Engine::get_instance()->get_clock()}});
#endif

    // remove execPtr from the vector since it finished
    simgrid::s4u::ExecPtr ep = pending_execs.at(index);
    pending_execs.erase(pending_execs.begin()+index);

    XBT_DEBUG("End of execution for %p, call output function", td);
    simgrid::s4u::ActorPtr out = simgrid::s4u::Actor::create(
      mbName_+"outputf"+boost::uuids::to_string(uuidGen_()), simgrid::s4u::Host::current(), [&]{outputFunction_(td);});
    if (keepGoing_)n_empty_->release();
    }
}

void TaskInstance::run() {
  host_ = simgrid::s4u::this_actor::get_host();

  poll = simgrid::s4u::Actor::create(
    mbName_+boost::uuids::to_string(uuidGen_()), simgrid::s4u::Host::current(), [&]{pollTasks();});
  pollEnd = simgrid::s4u::Actor::create(mbName_+"pollEnd",
    simgrid::s4u::Host::current(), [&]{pollEndOfTaskExec();});
  // boot duration (just sleep so that we don't process any request in the node until bootime elapsed)
  simgrid::s4u::this_actor::sleep_for(bootTime_);

  while (keepGoing_) {
    try {
      int n = n_full_->acquire_timeout(1000);
      if (!(n == 0))
        continue;

      if (reqs.size() == 0) {
        continue;
      }
      // receive data from mailbox
      TaskDescription* td = reqs.at(0);
      td->startExec = simgrid::s4u::Engine::get_clock();
      reqs.erase(reqs.begin());
      XBT_DEBUG("instance received req %p", td);
#ifdef USE_JAEGERTRACING
        std::string rName = etm_->getStrFromReqType(td->requestType);
        auto t1 = std::chrono::seconds(946684800)+std::chrono::microseconds(
            static_cast<int>(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
        auto span = td->parentSpans.size() == 0?
          etm_->getTracer().get()->StartSpan(rName,
            {opentracing::v3::StartTimestamp(t1)}) :
          etm_->getTracer().get()->StartSpan(rName,
            {opentracing::v3::StartTimestamp(t1), opentracing::v3::ChildOf(
              &td->parentSpans.at(td->parentSpans.size()-1)->get()->context())});
        span->Log({ {"nbInst:", etm_->getInstanceAmount()},
          {"waitingReqAtStart:", etm_->getAmountOfWaitingRequests()},
          {"alreadyExecutingAtStart:", etm_->getAmountOfExecutingRequests()},
          {"startExec", simgrid::s4u::Engine::get_instance()->get_clock()}});
        std::unique_ptr<opentracing::v3::Span>* t = new std::unique_ptr<opentracing::v3::Span>();
        *t = std::move(span);
        td->parentSpans.push_back(t);
#endif

        etm_->modifWaitingReqAmount(-1);
        etm_->modifExecutingReqAmount(1);

        /**
         * Here we start the execution of or request. 2 important properties:
         * - execution amount: gives the duration of the execution given the flop capacity of the machine.
         * E.G A 1GF node will have a 10^9 flop execution amount for a 1 second long request
         * - Execution ratio: used to adapt the execution amount to the amount of time spent actively using the CPU.
         * E.G A 1s long request with an exec ratio of .5 will spend .5sec cpu time and .5sec idle (in practice
         * not really, we just set a bound to the execution, but it has the same effect)
         */
        double execRatio = etm_->getExecRatio(td);
        double execAmount = etm_->getCPUExecAmount(td);
        double realToBeExecuted = execRatio * execAmount;
        double boundValue;
        if (execRatio < 1)
          boundValue = execRatio * simgrid::s4u::this_actor::get_host()->get_speed();
        else
          boundValue = simgrid::s4u::this_actor::get_host()->get_speed();
        XBT_DEBUG("Start execution of request. Flops: %lf, Ratio: %lf, CPUBound: %lf/%lf",
          realToBeExecuted, execRatio, boundValue, simgrid::s4u::this_actor::get_host()->get_speed());

        /* don't launch it directly since we want to set a bound*/
        simgrid::s4u::ExecPtr execP = simgrid::s4u::this_actor::exec_init(realToBeExecuted);
        // this_actor::exec_async(realToBeExecuted);
        execP->set_bound(boundValue);
        execP->start();

        // append the execPTr to the exec vector of executing requests,
        // it will be handled in the endExec polling actor once finished
        pending_execs.push_back(execP);
        execMap_.insert(std::pair<simgrid::s4u::ExecPtr, TaskDescription*>(execP, td));

        // allow the pollEnds to execute
        sem_pollExec_->release();
    } catch (std::exception e) {XBT_INFO("woops: %s", e.what());}
  }
}

void TaskInstance::kill() {
  XBT_INFO("Kill taskinstance td size: %d %d", reqs.size(), execMap_.size());

  keepGoing_ = false;
  // don't receive requests anymore
  // simgrid::s4u::Comm::wait_all(&commV);
  for (auto c : commV) {
    c->cancel();
  }

  // I GUESS IT IS NOT THE WAY TO DO
  // IF WE KILL THE INSTANCE BUT SOME EXEC_ASYNC ARE STILL EXECUTING
  // WAIT FOR THE LAST ONE TO FINISH
  // IT SEEMS TO AVOID A SEGFAULT IN THE HOSTLOAD PLUGIN, BUT ITS A HORRIBLE WAY TO DO IT I GUESS
  if (pending_execs.size() > 0)
    pending_execs.at(pending_execs.size()-1)->wait();

  poll->kill();
  pollEnd->kill();
  n_empty_->release();
  n_full_->release();
  sem_pollExec_->release();
}
}  // namespace sg_microserv
