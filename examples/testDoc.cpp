#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Host.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Comm.hpp>
#include "ElasticPolicy.hpp"
#include "ElasticTask.hpp"
#include "DataSource.hpp"
#include <memory>

XBT_LOG_NEW_DEFAULT_CATEGORY(run_log, "logs of the experiment");

using namespace sg_microserv;

/**
 * example command:
 * ./testDoc ../../platforms/dejavu_platform.xml --log=run_log.thres:debug
 * (dirty stopping of the async polling actors so small error when killing but it doesn't change the results)
 */

/**
 * Function called as the output of the first service
 * Redirects the output of etm1 to the second service
 */
void returnservice1(TaskDescription* td) {
	XBT_DEBUG("Return function of service service1");
	s4u_Mailbox* mservice2 = s4u_Mailbox::by_name("service2");
	mservice2->put(td, td->dSize);
}

/**
 * Function called as the output of the second service
 * it does nothing (acts as a sink)
 */
void returnservice2(TaskDescription* td) {
	XBT_DEBUG("Return function of service service2");
#ifdef USE_JAEGERTRACING
XBT_DEBUG("CLOSE SPANS");
	for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::milliseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});

  }
#endif /*USE_JAEGERTRACING*/
	// here you can put the msg to a sink if desired
}

void run() {
  /**
   * 2 ETMs (ETMs represent a service, with the manager that handles instances)
   *  - ETM1 "etmservice1":
   *    - 1 input mailbox for task polling named "service1"
   *    - 1 task = 1e7 flops (0.01s on a 1GF machine as the default one here)
   *    - the instance is supposed to be operational after 2 seconds
   *    - the output size will be the same as the input: used to model filtering and such after data processing
   *  - ETM2:
   *    - 1 input mailbox "service2"
   *    - 5e6 flops per task execution
   *    - output size = input size
   *    - 1 second boot time
   */

  /* ETM1 */
  std::vector<std::string> vservice1 = std::vector<std::string>();
  vservice1.push_back("service1");
  std::shared_ptr<sg_microserv::ElasticTaskManager> etmservice1 = std::make_shared<sg_microserv::ElasticTaskManager>("etmservice1",vservice1);

  etmservice1->setOutputFunction(returnservice1);
  /* 3 instances (not using an elastic policy here, see other examples) */
  etmservice1->addHost(simgrid::s4u::Host::by_name("cb1-2"));
  etmservice1->addHost(simgrid::s4u::Host::by_name("cb1-3"));
  etmservice1->addHost(simgrid::s4u::Host::by_name("cb1-4"));
  etmservice1->setExecAmount(1e7);
  etmservice1->setBootDuration(2);
  simgrid::s4u::Actor::create("etmservice1_a", simgrid::s4u::Host::by_name("cb1-1"), [etmservice1] { etmservice1->run(); });


  /* ETM2 */
  std::vector<std::string> vservice2 = std::vector<std::string>();
  vservice2.push_back("service2");
  std::shared_ptr<sg_microserv::ElasticTaskManager> etmservice2 = std::make_shared<sg_microserv::ElasticTaskManager>("etmservice2",vservice2);
  etmservice2->setOutputFunction(returnservice2);
  /* 2 instances */
  etmservice2->addHost(simgrid::s4u::Host::by_name("cb1-101"));
  etmservice2->addHost(simgrid::s4u::Host::by_name("cb1-102"));
  etmservice2->setExecAmount(5e6);
  etmservice2->setBootDuration(1);
  simgrid::s4u::Actor::create("etmservice2_a", simgrid::s4u::Host::by_name("cb1-100"), [etmservice2] { etmservice2->run(); });


  /*
    * Create data source
    * the data source will send requests to the first service
    * through its mailbox (service1) every .5 second, and each
    * request has size 10KB
    * Of course you can create as many data sources as you want
    * (and not necessarily with a fixed period)
    *
  */
  DataSourceFixedInterval* ds = new DataSourceFixedInterval("service1", 1, 10000);
	simgrid::s4u::ActorPtr dataS = simgrid::s4u::Actor::create("snd", simgrid::s4u::Host::by_name("cb1-1"), [&]{ds->run();});

  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(300);
  XBT_INFO("Done. Killing policies and etms");
  dataS->kill();
  etmservice1->kill();
  etmservice2->kill();
}


int main(int argc, char* argv[]) {
	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);
	e->load_platform(argv[1]);
	simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name("cb1-200"), [&]{run();});
	e->run();
	return 0;
}
