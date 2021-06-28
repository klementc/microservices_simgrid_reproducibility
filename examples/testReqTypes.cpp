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

void return_nginx_web_server(TaskDescription* td) {
	XBT_INFO("Return function of service nginx");
  switch (td->requestType)
  {
    case RequestType::TESTREQ_EXAMPLE:
      XBT_INFO("Outputfunction for TESTREQ_EXAMPLE, put to user_timeline");
    	s4u_Mailbox* m_user_timeline = s4u_Mailbox::by_name("user_timeline");
    	m_user_timeline->put(td, td->dSize);
      break;
  }

}

double pr_nginx_web_server(TaskDescription* td){
  XBT_INFO("pr function of service nginx");
  switch (td->requestType)
  {
    case RequestType::TESTREQ_EXAMPLE:
    	if(td->nbHops == 1){XBT_INFO("ret 5e6");return 5e6;}
      break;
  }
  XBT_INFO("PROBLEM");
  return 1;
}

void return_user_timeline(TaskDescription* td) {
	XBT_INFO("Return function of service timeline");
  switch (td->requestType)
  {
    case RequestType::TESTREQ_EXAMPLE:
      XBT_INFO("Outputfunction for TESTREQ_EXAMPLE, put to post_storage");
      s4u_Mailbox* m_post_storage = s4u_Mailbox::by_name("post_storage");
    	m_post_storage->put(td, td->dSize);
      break;
  }
}

double pr_user_timeline(TaskDescription* td){
  XBT_INFO("pr function of service timeline");
  switch (td->requestType)
  {
    case RequestType::TESTREQ_EXAMPLE:
    	if(td->nbHops == 2){XBT_INFO("ret 1e8");return 0;}
      break;
  }
  XBT_INFO("PROBLEM");
  return 1;
}

void return_post_storage(TaskDescription* td) {
	XBT_INFO("Return function of service post storage");
  switch (td->requestType)
  {
    case RequestType::TESTREQ_EXAMPLE:
      XBT_INFO("Outputfunction for TESTREQ_EXAMPLE, finished");
      delete td;
      break;
  }
}

double pr_post_storage(TaskDescription* td){
  XBT_INFO("pr function of service post_storage");
  switch (td->requestType)
  {
    case RequestType::TESTREQ_EXAMPLE:
    	if(td->nbHops == 3){XBT_INFO("ret 4e7");return 4e7;}
      break;
  }
  XBT_INFO("PROBLEM");
  return 1;
}

void run() {
  XBT_INFO("Starting run()");

  // create ETMs
  std::vector<std::string> v_serv_nginx_web_server = std::vector<std::string>();
  v_serv_nginx_web_server.push_back("nginx_web_server");
  std::shared_ptr<sg_microserv::ElasticTaskManager> serv_nginx_web_server = std::make_shared<sg_microserv::ElasticTaskManager>("nginx_web_server",v_serv_nginx_web_server);
  serv_nginx_web_server->setBootDuration(0);
  serv_nginx_web_server->setOutputFunction(return_nginx_web_server);
  serv_nginx_web_server->setExecAmountFunc(pr_nginx_web_server);
  serv_nginx_web_server->addHost(simgrid::s4u::Host::by_name("cb1-1"));

  std::vector<std::string> v_serv_user_timeline = std::vector<std::string>();
  v_serv_user_timeline.push_back("user_timeline");
  std::shared_ptr<sg_microserv::ElasticTaskManager> serv_user_timeline = std::make_shared<sg_microserv::ElasticTaskManager>("user_timeline",v_serv_user_timeline);
  serv_user_timeline->setBootDuration(0);
  serv_user_timeline->setOutputFunction(return_user_timeline);
  serv_user_timeline->setExecAmountFunc(pr_user_timeline);
  serv_user_timeline->addHost(simgrid::s4u::Host::by_name("cb1-2"));



  std::vector<std::string> v_serv_post_storage = std::vector<std::string>();
  v_serv_post_storage.push_back("post_storage");
  std::shared_ptr<sg_microserv::ElasticTaskManager> serv_post_storage = std::make_shared<sg_microserv::ElasticTaskManager>("post_storage",v_serv_post_storage);
  serv_post_storage->setBootDuration(0);
  serv_post_storage->setOutputFunction(return_post_storage);
  serv_post_storage->setExecAmountFunc(pr_post_storage);
  serv_post_storage->addHost(simgrid::s4u::Host::by_name("cb1-3"));



  // create actors
  simgrid::s4u::Actor::create("etm_nginx_web_server", simgrid::s4u::Host::by_name("cb1-1"), [serv_nginx_web_server] { serv_nginx_web_server->run(); });
  simgrid::s4u::Actor::create("etm_user_timeline", simgrid::s4u::Host::by_name("cb1-2"), [serv_user_timeline] { serv_user_timeline->run(); });
  simgrid::s4u::Actor::create("etm_post_storage", simgrid::s4u::Host::by_name("cb1-3"), [serv_post_storage] { serv_post_storage->run(); });


  /*create datasource*/
  DataSourceFixedInterval* dsf = new DataSourceFixedInterval("nginx_web_server",RequestType::TESTREQ_EXAMPLE, 5,100);
  simgrid::s4u::ActorPtr dataS = simgrid::s4u::Actor::create("snd", simgrid::s4u::Host::by_name("cb1-100"), [&]{dsf->run();});


  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(30);
  XBT_INFO("Done. Killing policies and etms");
  dataS->kill();
  serv_nginx_web_server->kill();
  serv_user_timeline->kill();
  serv_post_storage->kill();
}
int main(int argc, char* argv[]) {
	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);

	e->load_platform(argv[1]);
	simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name("cb1-200"), [&]{run();});
	e->run();
	return 0;
}
