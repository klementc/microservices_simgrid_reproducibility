
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

/* RETURN FUNCTIONS, AUTO GENERATED CODE, MODIFY IF YOU KNOW WHAT YOU WANT */
    void return_nginx_web_server(TaskDescription* td) {
      XBT_DEBUG("Return function of service nginx_web_server");
      switch (td->requestType)
      {
case RequestType::READHOME:
      if(td->nbHops == 1){
        XBT_DEBUG("Output Function for READHOME, put to home_timeline_service");
        s4u_Mailbox* m_user_timeline = s4u_Mailbox::by_name("home_timeline_service");
        m_user_timeline->put(td, td->dSize);
      }

break;

case RequestType::READUSER:
      if(td->nbHops == 1){
        XBT_DEBUG("Output Function for READUSER, put to user_timeline_service");
        s4u_Mailbox* m_user_timeline = s4u_Mailbox::by_name("user_timeline_service");
        m_user_timeline->put(td, td->dSize);
      }

break;
	}
}
    void return_home_timeline_service(TaskDescription* td) {
      XBT_DEBUG("Return function of service home_timeline_service");
      switch (td->requestType)
      {
case RequestType::READHOME:
      if(td->nbHops == 2){
        XBT_DEBUG("Output Function for READHOME, put to post_storage_service");
        s4u_Mailbox* m_user_timeline = s4u_Mailbox::by_name("post_storage_service");
        m_user_timeline->put(td, td->dSize);
      }

break;
	}
}
    void return_post_storage_service(TaskDescription* td) {
      XBT_DEBUG("Return function of service post_storage_service");
      switch (td->requestType)
      {
case RequestType::READHOME:
      if(td->nbHops == 3){
        XBT_DEBUG("Output Function for READHOME, Final service, DELETE");
        for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if((*it)->get() != NULL){
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }
        delete td;
        // TODO DELETE JAEGER SPANS
      }

break;

case RequestType::READUSER:
      if(td->nbHops == 3){
        XBT_DEBUG("Output Function for READUSER, Final service, DELETE");
        for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if((*it)->get() != NULL){
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }
        delete td;
        // TODO DELETE JAEGER SPANS
      }

break;
	}
}
    void return_user_timeline_service(TaskDescription* td) {
      XBT_DEBUG("Return function of service user_timeline_service");
      switch (td->requestType)
      {
case RequestType::READUSER:
      if(td->nbHops == 2){
        XBT_DEBUG("Output Function for READUSER, put to post_storage_service");
        s4u_Mailbox* m_user_timeline = s4u_Mailbox::by_name("post_storage_service");
        m_user_timeline->put(td, td->dSize);
      }

break;
	}
}
/* PR FUNCTIONS, AUTO GENERATED CODE, MODIFY IF YOU KNOW WHAT YOU WANT */
    double pr_nginx_web_server(TaskDescription* td) {
      XBT_DEBUG("pr function of service nginx_web_server");
      switch (td->requestType)
      {
case RequestType::READHOME:
      if(td->nbHops == 1){XBT_DEBUG("Entered cost Function for READHOME"); return 1564000;}

break;

case RequestType::READUSER:
      if(td->nbHops == 1){XBT_DEBUG("Entered cost Function for READUSER"); return 1879000;}

break;
	}//it should never end up here
return -1;
}
    double pr_home_timeline_service(TaskDescription* td) {
      XBT_DEBUG("pr function of service home_timeline_service");
      switch (td->requestType)
      {
case RequestType::READHOME:
      if(td->nbHops == 2){XBT_DEBUG("Entered cost Function for READHOME"); return 1142000;}

break;
	}//it should never end up here
return -1;
}
    double pr_post_storage_service(TaskDescription* td) {
      XBT_DEBUG("pr function of service post_storage_service");
      switch (td->requestType)
      {
case RequestType::READHOME:
      if(td->nbHops == 3){XBT_DEBUG("Entered cost Function for READHOME (FINAL NODE, NO CHILD!)"); return 17000;}

break;

case RequestType::READUSER:
      if(td->nbHops == 3){XBT_DEBUG("Entered cost Function for READUSER (FINAL NODE, NO CHILD!)"); return 17000;}

break;
	}//it should never end up here
return -1;
}
    double pr_user_timeline_service(TaskDescription* td) {
      XBT_DEBUG("pr function of service user_timeline_service");
      switch (td->requestType)
      {
case RequestType::READUSER:
      if(td->nbHops == 2){XBT_DEBUG("Entered cost Function for READUSER"); return 2355000;}

break;
	}//it should never end up here
return -1;
}
  void run() {
    XBT_INFO("Starting run()");

    // create ETM for service nginx_web_server
    std::vector<std::string> v_serv_nginx_web_server = std::vector<std::string>();
    v_serv_nginx_web_server.push_back("nginx_web_server");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_nginx_web_server = std::make_shared<sg_microserv::ElasticTaskManager>("nginx_web_server",v_serv_nginx_web_server);
    serv_nginx_web_server->setBootDuration(0);
    serv_nginx_web_server->setOutputFunction(return_nginx_web_server);
    serv_nginx_web_server->setExecAmountFunc(pr_nginx_web_server);
    serv_nginx_web_server->addHost(simgrid::s4u::Host::by_name("cb1-1"));
    simgrid::s4u::Actor::create("etm_nginx_web_server", simgrid::s4u::Host::by_name("cb1-1"), [serv_nginx_web_server] { serv_nginx_web_server->run(); });


    // create ETM for service home_timeline_service
    std::vector<std::string> v_serv_home_timeline_service = std::vector<std::string>();
    v_serv_home_timeline_service.push_back("home_timeline_service");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_home_timeline_service = std::make_shared<sg_microserv::ElasticTaskManager>("home_timeline_service",v_serv_home_timeline_service);
    serv_home_timeline_service->setBootDuration(0);
    serv_home_timeline_service->setOutputFunction(return_home_timeline_service);
    serv_home_timeline_service->setExecAmountFunc(pr_home_timeline_service);
    serv_home_timeline_service->addHost(simgrid::s4u::Host::by_name("cb1-2"));
    simgrid::s4u::Actor::create("etm_home_timeline_service", simgrid::s4u::Host::by_name("cb1-2"), [serv_home_timeline_service] { serv_home_timeline_service->run(); });


    // create ETM for service post_storage_service
    std::vector<std::string> v_serv_post_storage_service = std::vector<std::string>();
    v_serv_post_storage_service.push_back("post_storage_service");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_post_storage_service = std::make_shared<sg_microserv::ElasticTaskManager>("post_storage_service",v_serv_post_storage_service);
    serv_post_storage_service->setBootDuration(0);
    serv_post_storage_service->setOutputFunction(return_post_storage_service);
    serv_post_storage_service->setExecAmountFunc(pr_post_storage_service);
    serv_post_storage_service->addHost(simgrid::s4u::Host::by_name("cb1-3"));
    simgrid::s4u::Actor::create("etm_post_storage_service", simgrid::s4u::Host::by_name("cb1-3"), [serv_post_storage_service] { serv_post_storage_service->run(); });


    // create ETM for service user_timeline_service
    std::vector<std::string> v_serv_user_timeline_service = std::vector<std::string>();
    v_serv_user_timeline_service.push_back("user_timeline_service");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_user_timeline_service = std::make_shared<sg_microserv::ElasticTaskManager>("user_timeline_service",v_serv_user_timeline_service);
    serv_user_timeline_service->setBootDuration(0);
    serv_user_timeline_service->setOutputFunction(return_user_timeline_service);
    serv_user_timeline_service->setExecAmountFunc(pr_user_timeline_service);
    serv_user_timeline_service->addHost(simgrid::s4u::Host::by_name("cb1-4"));
    simgrid::s4u::Actor::create("etm_user_timeline_service", simgrid::s4u::Host::by_name("cb1-4"), [serv_user_timeline_service] { serv_user_timeline_service->run(); });



  /* ADD DATASOURCES MANUALLY HERE, SET THE END TIMER AS YOU WISH, AND LAUNCH YOUR SIMULATOR*/
  DataSourceFixedInterval* dsf = new DataSourceFixedInterval("nginx_web_server",RequestType::READUSER, 30,100);
  simgrid::s4u::ActorPtr dataS = simgrid::s4u::Actor::create("snd", simgrid::s4u::Host::by_name("cb1-100"), [&]{dsf->run();});

  DataSourceFixedInterval* dsf2 = new DataSourceFixedInterval("nginx_web_server",RequestType::READHOME, 51,100);
  simgrid::s4u::ActorPtr dataS2 = simgrid::s4u::Actor::create("snd", simgrid::s4u::Host::by_name("cb1-100"), [&]{dsf2->run();});

  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(150); /*set it according to your needs*/
  XBT_INFO("Done. Killing policies and etms");
    dataS->kill();
    dataS2->kill();
  // kill policies and ETMs
  simgrid::s4u::this_actor::sleep_for(150); /*set it according to your needs*/
  XBT_INFO("Done. Killing policies and etms");

    serv_nginx_web_server->kill();
    serv_home_timeline_service->kill();
    serv_post_storage_service->kill();
    serv_user_timeline_service->kill();
}
int main(int argc, char* argv[]) {
	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);

	e->load_platform(argv[1]);
	simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name("cb1-200"), [&]{run();});
	e->run();
	return 0;
}
