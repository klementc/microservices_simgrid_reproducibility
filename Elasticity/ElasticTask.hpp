#ifndef ELASTICITY_ELASTICTASK_HPP_
#define ELASTICITY_ELASTICTASK_HPP_

#include <vector>
#include <queue>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <memory>


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <xbt/string.hpp>
#include <xbt/signal.hpp>
#include <xbt/Extendable.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Io.hpp>

#include "ElasticConfig.hpp"
#include "RequestType.hpp"

namespace sg_microserv {

class EvntQ {
 public:
    double date;

    explicit EvntQ(double date_) : date(date_) {}
    virtual ~EvntQ() {}
};

/**
 * Compare EvntQ's event timestamps
 */
struct Comparator {
  bool operator()(const EvntQ* lhs, const EvntQ* rhs) {
    return lhs->date > rhs->date;
  }
};

class TaskDescription : public EvntQ {
 public:
    boost::uuids::uuid id_;
    int nbHops;
    double firstArrivalDate;
    bool repeat = true;
    std::function<void()> outputFunction = []() {};
    bool hasTimestamps = false;
    double dSize;
    double startExec;
    double queueArrival;
    double instArrival;
    double endExec;
    bool finished;
    // to enable multi routes (the important part will be in the output function, responible
    // for sending to output mailboxes depending ont the request type)
    RequestType requestType;
    // if ack needed (ex: waiting for data from a DB before sending a request to the next service)
    // then in the output function: push a mb name on the stack, send requests to the services you want an ack for
    // those services in their return function will pop the mb from the stack and send an ack
    // then the original service, after receiving the ACK, can send a req to the next services
    // THUS, an important thing to note is that the routing part is
    // dependent on the implenentation of the output services functions
    std::stack<std::string> ackStack;
    std::vector<double> flopsPerServ;

#ifdef USE_JAEGERTRACING
    std::vector<std::unique_ptr<opentracing::v3::Span>*> parentSpans;
#endif

    TaskDescription(boost::uuids::uuid id, double date_, double dSize_, RequestType requestType_)
        : EvntQ(date_), id_(id), dSize(dSize_), requestType(requestType_),
        nbHops(0), finished(-1), firstArrivalDate(-1), startExec(-1),
        queueArrival(-1), instArrival(-1), endExec(-1) {}

    TaskDescription(boost::uuids::uuid id, double date_)
        : TaskDescription(id,  date_, -1, RequestType::DEFAULT) {
    }

    explicit TaskDescription(boost::uuids::uuid id)
      : TaskDescription(id, 0.0) {}
};

// namespace simgrid {
// namespace s4u {

class TaskInstance;

/** @brief */
class ElasticTaskManager {
 private:
    /* The function to call at the end of an instance's execution */
    std::function<void(TaskDescription*)> outputFunction = [](TaskDescription*) {};

    /* default cpu cost in case fCPUCost_ is not defined (all request will execute this default cost) */
    int64_t defCPUCost_;
    /* match request type to request cost */
    std::function<double(TaskDescription*)> fCPUCost;

    /* default cpu ratio in case fCPURatio_ is not defined (all requests will use this default ratio) */
    double defCPURatio_;
    std::function<double(TaskDescription*)> fCPURatio_;

    /* Hosts that contain an instance */
    std::vector<simgrid::s4u::Host*> availableHostsList_;
    std::vector<TaskInstance*> tiList;

    /* transforms request type into string for debugging purposes*/
    std::function<std::string(RequestType)> reqTypeToStr_;

    /* ETM properties */
    std::string serviceName_;
    std::vector<std::string> incMailboxes_;
    bool keepGoing;
    int nextHost_;
    double bootDuration_;
    int parallelTasksPerInst_;

    /*
     * used for requests to wait for all dependencies to be received within a graph node
     * (respect precedence constraints and don't execute a task before all it's data arrived)
     */
    std::map<boost::uuids::uuid, std::vector<TaskDescription*>> tempData;

    // list of triggered TDs
    std::priority_queue<EvntQ*, std::vector<EvntQ*>, Comparator> nextEvtQueue;
    simgrid::s4u::SemaphorePtr sleep_sem;

    simgrid::s4u::SemaphorePtr modif_sem_;
    boost::uuids::random_generator uuidGen_;

    /* metrics */
    int64_t waitingReqAmount_;
    int64_t executingReqAmount_;
    int counterExecSlot_;

    std::vector<simgrid::s4u::ActorPtr> pollers_;
#ifdef USE_JAEGERTRACING
    std::shared_ptr<opentracing::v3::Tracer> tracer_;
#endif
    // for async send to taskInstances
    std::vector<simgrid::s4u::CommPtr> pending_comms;

 public:
    /* request type to string (very stupid, should be modified) */
    ElasticTaskManager(std::string name, std::vector<std::string> incMailboxes, std::string jaegConfigFile);
    ElasticTaskManager(std::string name, std::vector<std::string> incMailboxes);
    explicit ElasticTaskManager(std::string name);

    /* fetch new task requests*/
    void pollnet(std::string mbName);
    /* create a new instance on the given host*/
    void addHost(simgrid::s4u::Host *host);
    /* remove an instance */
    simgrid::s4u::Host* removeHost(int i);

    /* modify flops amount per req
     * if func not set, then the setExecAmountn value will be taken for every request
     * if func is set, the setExecAmount func will always be used
     * */
    void setExecAmount(int64_t pr);
    void setExecAmountFunc(std::function<double(TaskDescription*)> costReqType);

    /**
     * Set the default amount of IO for a request
     * if func not set, then the defaultIOCost will be used for all requests
     * if func is set, the function will always be used to determine the amount of IO to be performed
     */
    void setExecRatio(double ior);
    void setExecRatioFunction(std::function<double(TaskDescription*)> td);

    /* obtain the processing cost for a given request type */
    double getCPUExecAmount(TaskDescription*);

    /* obtain the execution ratio used to set a cpu bound on executions that don't
     * only consist in CPU executions (could be replaced in the future by real I/Os)
     */
    double getExecRatio(TaskDescription*);

    /* duration between instance creation and operational state*/
    void setBootDuration(double bd);

    /* ETM properties*/
    int64_t getAmountOfWaitingRequests();
    int64_t getAmountOfExecutingRequests();
    void modifExecutingReqAmount(int n);
    void modifWaitingReqAmount(int n);
    void setParallelTasksPerInst(int s);

    /* trigger a taskdescription*/
    void trigger(TaskDescription* td);
    void setOutputFunction(std::function<void(TaskDescription*)> code);

    void setReqNames(std::function<std::string(RequestType)> rnToStr);

    void kill();
    void run();

    std::string getStrFromReqType(RequestType);

    // basic metrics
    std::vector<double> getCPULoads();
    unsigned int getInstanceAmount();
    inline int getCounterExecSlot() {return counterExecSlot_;}
    inline void setCounterExecSlot(int v) {counterExecSlot_ = v;}
    inline void resetCounterExecSlot() {counterExecSlot_ = 0;}
    inline std::string getServiceName() {return serviceName_;}
    double reqPerSec();

    ~ElasticTaskManager();

#ifdef USE_JAEGERTRACING
    /**
     * Obtain the tracer creater for jaeger interception
     * Only used if the jaegertracing option is ON
     */
    std::shared_ptr<opentracing::v3::Tracer> getTracer();
#endif
};


class TaskInstance {
 private:
    bool keepGoing_;
    std::function<void(TaskDescription*)> outputFunction_ = [](TaskDescription*) {};
    ElasticTaskManager* etm_;
    std::string mbName_;

    simgrid::s4u::SemaphorePtr n_empty_;
    simgrid::s4u::SemaphorePtr n_full_;
    // semaphore used to know if some executions take place
    simgrid::s4u::SemaphorePtr sem_pollExec_;


    double bootTime_;
    boost::uuids::random_generator uuidGen_;
    simgrid::s4u::ActorPtr poll, pollEnd;

    int maxReqInInst_;
    std::vector<TaskDescription*> reqs;
    simgrid::s4u::Host* host_;
    std::vector<simgrid::s4u::CommPtr> commV;

    // structures used to perform CPU executions
    std::vector<simgrid::s4u::ExecPtr> pending_execs;
    std::map<simgrid::s4u::ExecPtr, TaskDescription*> execMap_;

    /** disk to use for I/Os and structures for pending I/O
     * IMPORTANT: REPLACED BY A CPU BOUND FOR NOW TO AVOID TOO COMPLEX  CALIBRATION ISSUES
     * simgrid::s4u::Disk* disk_;
     * std::vector<simgrid::s4u::IoPtr> io_vector;
     * std::map<simgrid::s4u::IoPtr, TaskDescription*> ioMap_;
     * // semaphore used to know if some IO operations take place
     * simgrid::s4u::SemaphorePtr sem_pollIO_;
     * // actor charged of managing I/O executions
     * void pollEndOfTaskIO();
     */

    /**
     * Received and intiatesthe IO/CPU executions
    */
    void pollTasks();

    /**
     * actor charged of managing CPU executions
    */
    void pollEndOfTaskExec();

 public:
    TaskInstance(ElasticTaskManager* etm, std::string mbName,
      std::function<void(TaskDescription*)> outputFunction,
      int maxReqInst, double bootTime);
    TaskInstance(ElasticTaskManager* etm, std::string mbName,
      std::function<void(TaskDescription*)> outputFunction,
      int maxReqInst);


    void run();
    void kill();
    inline simgrid::s4u::Host* getRunningHost() {return host_;}
};

}  // namespace sg_microserv

#endif  // ELASTICITY_ELASTICTASK_HPP_
