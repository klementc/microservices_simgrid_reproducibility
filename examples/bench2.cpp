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
  XBT_INFO("output service 1 queueArrival: %f instArrival: %f startExec: %f endExec: %f s1cost: %f name: service1\n", td->queueArrival, td->instArrival, td->startExec, td->endExec, td->flopsPerServ.at(0));
	//XBT_INFO("Return function of service service1");
	s4u_Mailbox* mservice2 = s4u_Mailbox::by_name("service2");
  //XBT_INFO("send %p to service2", td);
	mservice2->put(td, td->dSize);
}

/**
 * Function called as the output of the first service
 * Redirects the output of etm2 to the sink service
 */
void returnservice2(TaskDescription* td) {
  XBT_INFO("output service 2 queueArrival: %f instArrival: %f startExec: %f endExec: %f s1cost: %f name: service2\n", td->queueArrival, td->instArrival, td->startExec, td->endExec, td->flopsPerServ.at(1));
	//XBT_INFO("Return function of service service1");
	s4u_Mailbox* mservice3 = s4u_Mailbox::by_name("service3");
  //XBT_INFO("send %p to service2", td);
	mservice3->put(td, td->dSize);
}

/**
 * Function called as the output of the sink
 * it does nothing (acts as a sink)
 */
void returnservice3(TaskDescription* td) {
  //XBT_INFO("delete %p", td);
	//XBT_INFO("Return function of service service2");
#ifdef USE_JAEGERTRACING
XBT_DEBUG("CLOSE SPANS");
	for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();

  }

    if(td!=NULL){
    delete td;
    td = NULL;
  }
#endif /*USE_JAEGERTRACING*/
	// here you can put the msg to a sink if desired
}

void run(int serv1Flops, int serv2Flops, std::string tsFile, int parDeg) {
  std::cout << "Service1 with " << serv1Flops << " flops, Service2 with " << serv2Flops << " flops" <<std::endl;
  /* ETM1 */
  std::vector<std::string> vservice1 = std::vector<std::string>();
  vservice1.push_back("service1");
  std::shared_ptr<sg_microserv::ElasticTaskManager> etmservice1 = std::make_shared<sg_microserv::ElasticTaskManager>("etmservice1",vservice1);

  etmservice1->setOutputFunction(returnservice1);
  etmservice1->setParallelTasksPerInst(parDeg);
  simgrid::s4u::Actor::create("etmservice1_a", simgrid::s4u::Host::by_name("cb1-1"), [etmservice1] { etmservice1->run(); });
  /* 3 instances (not using an elastic policy here, see other examples) */
  etmservice1->addHost(simgrid::s4u::Host::by_name("cb1-2"));
  etmservice1->setExecAmount(serv1Flops);
  etmservice1->setBootDuration(0);

  /* ETM2 */
  std::vector<std::string> vservice2 = std::vector<std::string>();
  vservice2.push_back("service2");

  std::shared_ptr<sg_microserv::ElasticTaskManager> etmservice2 = std::make_shared<sg_microserv::ElasticTaskManager>("etmservice2",vservice2);
  etmservice2->setParallelTasksPerInst(parDeg);
  etmservice2->setOutputFunction(returnservice2);
  simgrid::s4u::Actor::create("etmservice2_a", simgrid::s4u::Host::by_name("cb1-100"), [etmservice2] { etmservice2->run(); });
  /* 2 instances */
  etmservice2->addHost(simgrid::s4u::Host::by_name("cb1-101"));
  etmservice2->setExecAmount(serv2Flops);
  etmservice2->setBootDuration(0);


  /* ETM3 (sink) */
  std::vector<std::string> vservice3 = std::vector<std::string>();
  vservice3.push_back("service3");
  std::shared_ptr<sg_microserv::ElasticTaskManager> etmservice3 = std::make_shared<sg_microserv::ElasticTaskManager>("etmservice3",vservice3);

  etmservice3->setOutputFunction(returnservice3);
  etmservice3->setParallelTasksPerInst(parDeg);
  simgrid::s4u::Actor::create("etmservice1_a", simgrid::s4u::Host::by_name("cb1-3"), [etmservice3] { etmservice3->run(); });
  /* 3 instances (not using an elastic policy here, see other examples) */
  etmservice3->addHost(simgrid::s4u::Host::by_name("cb1-4"));
  etmservice3->setExecAmount(0);
  etmservice3->setBootDuration(0);

  /*
    * Create data source
  */
  //DataSourceFixedInterval* ds = new DataSourceFixedInterval("service1", .5, 10000);
  //DataSourceTSFile* ds = new DataSourceTSFile("service1", "default5TimeStamps.csv", 1000);
  DataSourceTSFile* ds = new DataSourceTSFile("service1", tsFile, 1000);
	simgrid::s4u::ActorPtr dataS = simgrid::s4u::Actor::create("snd", simgrid::s4u::Host::by_name("cb1-1"), [&]{ds->run();});

  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(350);
  XBT_INFO("Done. Killing policies and etms");
  ds->suspend();
  dataS->kill();
  etmservice1->kill();
  etmservice2->kill();
  etmservice3->kill();
}


int main(int argc, char* argv[]) {
  	if(argc <= 5){
		std::cout << "required parameters:  <platform-file> <service1flops> <service2flops> <tsFile> <parDeg>" << std::endl;
		exit(1);
	}
	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);
	e->load_platform(argv[1]);

	simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name("cb1-200"), [&]{run(std::stoi(argv[2]), std::stoi(argv[3]), argv[4], std::stoi(argv[5]));});
	e->run();
	return 0;
}
