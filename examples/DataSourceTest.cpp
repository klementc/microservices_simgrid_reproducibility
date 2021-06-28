#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Host.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Comm.hpp>

#include "ElasticPolicy.hpp"
#include "ElasticTask.hpp"
#include "DataSource.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(run_log, "logs of the test");

using namespace sg_microserv;

void receiver(){
  s4u_Mailbox* mb = s4u_Mailbox::by_name("testOut");
  while(simgrid::s4u::Engine::get_clock() < 150){
    XBT_INFO("receiving");
    mb->get<TaskDescription*>();
    XBT_INFO("received");
  }
  simgrid::s4u::Actor::kill_all();
}
int main(int argc, char* argv[]) {
	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);
	e->load_platform(argv[1]);

  DataSourceFixedInterval* ds = new DataSourceFixedInterval("testOut", 10, 10000);
	simgrid::s4u::Actor::create("snd", simgrid::s4u::Host::by_name("cb1-1"), [&]{ds->run();});
  simgrid::s4u::Actor::create("rec", simgrid::s4u::Host::by_name("cb1-1"), [&]{receiver();});
	e->run();
	return 0;
}