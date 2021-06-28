#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Host.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Comm.hpp>
#include "ElasticTask.hpp"
#include "DataSource.hpp"
#include <memory>

#define EXEC_RATIO 0.5
#define EXEC_COST 1e9

XBT_LOG_NEW_DEFAULT_CATEGORY(run_log, "logs of the experiment");

using namespace sg_microserv;

/**
 * Function called as the output of the service
 * it does nothing (acts as a sink)
 */
void returnservice1(TaskDescription* td) {
  XBT_INFO("Execution: cost=%lf, Started=%lf, ended=%lf ratio=%lf",
    EXEC_COST, td->startExec, td->endExec, EXEC_RATIO);
  for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it) {
    if ((*it)->get() != NULL) {
      auto t2 = std::chrono::seconds(946684800) + std::chrono::microseconds(
        static_cast<int>(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
      (*it)->get()->Log({{"end", t2.count()}});
      (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
      (*it)->reset();
      (*it) == NULL;
    }
  }
        delete td;
}

void run() {


  /* ETM1 */
  std::vector<std::string> vservice1 = std::vector<std::string>();
  vservice1.push_back("service1");
  std::shared_ptr<ElasticTaskManager> etmservice1 = std::make_shared<ElasticTaskManager>("etmservice1",vservice1);

  etmservice1->setOutputFunction(returnservice1);
  etmservice1->setExecAmount(EXEC_COST);
  etmservice1->setParallelTasksPerInst(10);

  // only half of the work will be executed, the rest will be IDLE work
  // (setting a bound by dividing by 2 the cpu capacity)
  etmservice1->setExecRatio(EXEC_RATIO);
  etmservice1->setBootDuration(0);
  etmservice1->addHost(simgrid::s4u::Host::by_name("cb1-1"));
  simgrid::s4u::Actor::create("etmservice1_a",
    simgrid::s4u::Host::by_name("cb1-1"), [etmservice1] { etmservice1->run(); });

  /*
  * Create data source
  */
  DataSourceFixedInterval* dsf = new DataSourceFixedInterval("service1", RequestType::DEFAULT, 1, 100);
  simgrid::s4u::ActorPtr dataS = simgrid::s4u::Actor::create(
    "snd", simgrid::s4u::Host::by_name("cb1-1"), [&]{dsf->run();});

  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(5);
  XBT_INFO("Done. Killing policies and etms");
  dataS->kill();
  etmservice1->kill();
}


int main(int argc, char* argv[]) {
  simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);
  e->load_platform(argv[1]);
  simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name("cb1-2"), [&]{run();});
  e->run();
  return 0;
}
