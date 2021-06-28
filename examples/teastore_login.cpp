
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

int outSizes(RequestType, std::string);
/* RETURN FUNCTIONS, AUTO GENERATED CODE, MODIFY IF YOU KNOW WHAT YOU WANT */
    void return_REQ(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service REQ");
      switch (td->requestType)
      {
case RequestType::LOGIN:
      if (td->nbHops == 1) {
        XBT_DEBUG("Output Function for REQ (PARALLEL CHILDS)");
        std::vector<simgrid::s4u::CommPtr> comV;
        simgrid::s4u::CommPtr com;

          XBT_DEBUG("Put to servA");
          s4u_Mailbox* m_servA = s4u_Mailbox::by_name("servA");
          sg_microserv::TaskDescription* td_0 = new sg_microserv::TaskDescription(*td);
          td_0->requestType = RequestType::LOGIN_0;
          com = m_servA->put_async(td_0, outSizes(td_0->requestType, "servA"));
          comV.push_back(com);

          XBT_DEBUG("Put to servA2");
          s4u_Mailbox* m_servA2 = s4u_Mailbox::by_name("servA2");
          sg_microserv::TaskDescription* td_1 = new sg_microserv::TaskDescription(*td);
          td_1->requestType = RequestType::LOGIN_1;
          com = m_servA2->put_async(td_1, outSizes(td_1->requestType, "servA2"));
          comV.push_back(com);

simgrid::s4u::Comm::wait_all(&comV); // wait for communications to finish
break;
}
	}
}
    void return_servA(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servA");
      switch (td->requestType)
      {
case RequestType::LOGIN_0:
      if (td->nbHops == 2) {
        XBT_DEBUG("Output Function for servA (PARALLEL CHILDS)");
        std::vector<simgrid::s4u::CommPtr> comV;
        simgrid::s4u::CommPtr com;

          XBT_DEBUG("Put to servB");
          s4u_Mailbox* m_servB = s4u_Mailbox::by_name("servB");
          sg_microserv::TaskDescription* td_0 = new sg_microserv::TaskDescription(*td);
          td_0->requestType = RequestType::LOGIN_0_0;
          com = m_servB->put_async(td_0, outSizes(td_0->requestType, "servB"));
          comV.push_back(com);

          XBT_DEBUG("Put to servD");
          s4u_Mailbox* m_servD = s4u_Mailbox::by_name("servD");
          sg_microserv::TaskDescription* td_1 = new sg_microserv::TaskDescription(*td);
          td_1->requestType = RequestType::LOGIN_0_1;
          com = m_servD->put_async(td_1, outSizes(td_1->requestType, "servD"));
          comV.push_back(com);

          XBT_DEBUG("Put to servC");
          s4u_Mailbox* m_servC = s4u_Mailbox::by_name("servC");
          sg_microserv::TaskDescription* td_2 = new sg_microserv::TaskDescription(*td);
          td_2->requestType = RequestType::LOGIN_0_2;
          com = m_servC->put_async(td_2, outSizes(td_2->requestType, "servC"));
          comV.push_back(com);

simgrid::s4u::Comm::wait_all(&comV); // wait for communications to finish
break;
}
	}
}
    void return_servB(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servB");
      switch (td->requestType)
      {

case RequestType::LOGIN_0_0:
    if (td->nbHops == 3) {
      XBT_DEBUG("Output Function servB (FINAL NODE, NO CHILD!)");
      // TODO DELETE
 /*             for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }*/
    }

break;
	}
}
    void return_servD(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servD");
      switch (td->requestType)
      {

case RequestType::LOGIN_0_1:
    if (td->nbHops == 3) {
      XBT_DEBUG("Output Function servD (FINAL NODE, NO CHILD!)");
      // TODO DELETE
 /*             for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }*/
    }

break;
	}
}
    void return_servC(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servC");
      switch (td->requestType)
      {

case RequestType::LOGIN_0_2:
    if (td->nbHops == 3) {
      XBT_DEBUG("Output Function servC (FINAL NODE, NO CHILD!)");
      // TODO DELETE
             /* for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }*/
    }

break;
	}
}
    void return_servA2(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servA2");
      switch (td->requestType)
      {
case RequestType::LOGIN_1:
      if (td->nbHops == 2) {
        XBT_DEBUG("Output Function for LOGIN_1, put to servC2");
        s4u_Mailbox* m_servC2 = s4u_Mailbox::by_name("servC2");
        m_servC2->put(td, outSizes(td->requestType, "servC2"));


break;
}


case RequestType::LOGIN_1_1:
      if (td->nbHops == 4) {
        XBT_DEBUG("Output Function for LOGIN_1_1, put to servD2");
        s4u_Mailbox* m_servD2 = s4u_Mailbox::by_name("servD2");
        m_servD2->put(td, outSizes(td->requestType, "servD2"));


break;
}

break;
	}
}
    void return_servC2(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servC2");
      switch (td->requestType)
      {
case RequestType::LOGIN_1:
      if (td->nbHops == 3) {
        XBT_DEBUG("Output Function for servC2 (PARALLEL CHILDS)");
        std::vector<simgrid::s4u::CommPtr> comV;
        simgrid::s4u::CommPtr com;

          XBT_DEBUG("Put to servC2");
          s4u_Mailbox* m_servC2 = s4u_Mailbox::by_name("servC2");
          sg_microserv::TaskDescription* td_0 = new sg_microserv::TaskDescription(*td);
          td_0->requestType = RequestType::LOGIN_1_0;
          com = m_servC2->put_async(td_0, outSizes(td_0->requestType, "servC2"));
          comV.push_back(com);

          XBT_DEBUG("Put to servA2");
          s4u_Mailbox* m_servA2 = s4u_Mailbox::by_name("servA2");
          sg_microserv::TaskDescription* td_1 = new sg_microserv::TaskDescription(*td);
          td_1->requestType = RequestType::LOGIN_1_1;
          com = m_servA2->put_async(td_1, outSizes(td_1->requestType, "servA2"));
          comV.push_back(com);

          XBT_DEBUG("Put to servC2");
          s4u_Mailbox* m_servC2_ = s4u_Mailbox::by_name("servC2");
          sg_microserv::TaskDescription* td_2 = new sg_microserv::TaskDescription(*td);
          td_2->requestType = RequestType::LOGIN_1_2;
          com = m_servC2_->put_async(td_2, outSizes(td_2->requestType, "servC2"));
          comV.push_back(com);

simgrid::s4u::Comm::wait_all(&comV); // wait for communications to finish
break;
}



case RequestType::LOGIN_1_0:
    if (td->nbHops == 4) {
      XBT_DEBUG("Output Function servC2 (FINAL NODE, NO CHILD!)");
       XBT_INFO("FINISHED REQUEST at ts %lf arr: %lf dur: %lf", simgrid::s4u::Engine::get_clock(), td->firstArrivalDate, simgrid::s4u::Engine::get_clock()-td->firstArrivalDate);
      // TODO DELETE
    /*          for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }*/
    }

break;



case RequestType::LOGIN_1_2:
    if (td->nbHops == 4) {
      XBT_DEBUG("Output Function servC2 (FINAL NODE, NO CHILD!)");
      // TODO DELETE
   /*           for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }*/
    }

break;
	}
}
    void return_servD2(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("Return function of service servD2");
      switch (td->requestType)
      {
case RequestType::LOGIN_1_1:
      if (td->nbHops == 5) {
   /*     XBT_INFO("FINISHED REQUEST at ts %lf arr: %lf dur: %lf", simgrid::s4u::Engine::get_clock(), td->firstArrivalDate, simgrid::s4u::Engine::get_clock()-td->firstArrivalDate);
        XBT_DEBUG("Output Function for LOGIN_1_1, Final service, DELETE");
        for (auto it = td->parentSpans.rbegin(); it != td->parentSpans.rend(); ++it)
  {
    if ((*it)->get() != NULL) {
    auto t2 = std::chrono::seconds(946684800)+std::chrono::microseconds(int(simgrid::s4u::Engine::get_instance()->get_clock()*1000000));
    (*it)->get()->Log({{"end",t2.count()}});
    (*it)->get()->Finish({opentracing::v3::FinishTimestamp(t2)});
    (*it)->reset();
    (*it) == NULL;
    }
  }*/
        //delete td;
      }

break;
	}
}
/* PR FUNCTIONS, AUTO GENERATED CODE, MODIFY IF YOU KNOW WHAT YOU WANT */
    double pr_REQ(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service REQ");
      switch (td->requestType) {
case RequestType::LOGIN:
      if (td->nbHops == 1) {XBT_DEBUG("Entered cost Function for REQ"); return 0;
}
break;

      	}//it should never end up here
return -1;
}
    double pr_servA(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servA");
      switch (td->requestType) {
case RequestType::LOGIN_0:
      if (td->nbHops == 2) {XBT_DEBUG("Entered cost Function for servA"); return 7043000;
}
break;

      	}//it should never end up here
return -1;
}
    double pr_servB(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servB");
      switch (td->requestType) {

case RequestType::LOGIN_0_0:
    if (td->nbHops == 3) {XBT_DEBUG("Entered cost Function for servB"); return 57000;}

break;
	}//it should never end up here
return -1;
}
    double pr_servD(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servD");
      switch (td->requestType) {

case RequestType::LOGIN_0_1:
    if (td->nbHops == 3) {XBT_DEBUG("Entered cost Function for servD"); return 189000;}

break;
	}//it should never end up here
return -1;
}
    double pr_servC(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servC");
      switch (td->requestType) {

case RequestType::LOGIN_0_2:
    if (td->nbHops == 3) {XBT_DEBUG("Entered cost Function for servC"); return 542000;}

break;
	}//it should never end up here
return -1;
}
    double pr_servA2(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servA2");
      switch (td->requestType) {
case RequestType::LOGIN_1:
      if (td->nbHops == 2) {XBT_DEBUG("Entered cost Function for LOGIN_1"); return 13891000;}


case RequestType::LOGIN_1_1:
      if (td->nbHops == 4) {XBT_DEBUG("Entered cost Function for LOGIN_1_1"); return 828000;}

break;
	}//it should never end up here
return -1;
}
    double pr_servC2(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servC2");
      switch (td->requestType) {
case RequestType::LOGIN_1:
      if (td->nbHops == 3) {XBT_DEBUG("Entered cost Function for servC2"); return 6073000;
}
break;




case RequestType::LOGIN_1_0:
    if (td->nbHops == 4) {XBT_DEBUG("Entered cost Function for servC2"); return 4985000;}

break;



case RequestType::LOGIN_1_2:
    if (td->nbHops == 4) {XBT_DEBUG("Entered cost Function for servC2"); return 443000;}

break;
	}//it should never end up here
return -1;
}
    double pr_servD2(sg_microserv::TaskDescription* td) {
      XBT_DEBUG("pr function of service servD2");
      switch (td->requestType) {
case RequestType::LOGIN_1_1:
      if (td->nbHops == 5) {XBT_DEBUG("Entered cost Function for LOGIN_1_1 (FINAL NODE, NO CHILD!)"); return 408000;}

break;
	}//it should never end up here
return -1;
}
int outSizes(RequestType t, std::string nextServ) {
    switch (t) {
	case RequestType::LOGIN :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_0 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_0_0 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_0_1 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_0_2 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_1 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_1_0 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_1_1 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
	case RequestType::LOGIN_1_2 :
	if (nextServ == "servA") {return 100;}
	if (nextServ == "servB") {return 100;}
	if (nextServ == "servD") {return 100;}
	if (nextServ == "servC") {return 100;}
	if (nextServ == "servA2") {return 100;}
	if (nextServ == "servC2") {return 100;}
	if (nextServ == "servD2") {return 100;}
	break;
}return -1; // WE SHOULD NEVER GET TO THAT POINT
}

std::string reqTypeToStr(RequestType t) {
    switch (t) {
	case RequestType::LOGIN : return "LOGIN";break;
	case RequestType::LOGIN_0 : return "LOGIN_0";break;
	case RequestType::LOGIN_0_0 : return "LOGIN_0_0";break;
	case RequestType::LOGIN_0_1 : return "LOGIN_0_1";break;
	case RequestType::LOGIN_0_2 : return "LOGIN_0_2";break;
	case RequestType::LOGIN_1 : return "LOGIN_1";break;
	case RequestType::LOGIN_1_0 : return "LOGIN_1_0";break;
	case RequestType::LOGIN_1_1 : return "LOGIN_1_1";break;
	case RequestType::LOGIN_1_2 : return "LOGIN_1_2";break;
}return "WTF";
}

  void run(std::map<std::string, std::vector<std::string>> configServices, double freq) {
    XBT_INFO("Starting run()");

    // create ETM for service REQ
    std::vector<std::string> v_serv_REQ = std::vector<std::string>();
    v_serv_REQ.push_back("REQ");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_REQ = std::make_shared<sg_microserv::ElasticTaskManager>("REQ",v_serv_REQ);
    serv_REQ->setBootDuration(0);
    // serv_REQ->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_REQ->setParallelTasksPerInst(std::stoi(configServices.find("REQ")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_REQ->setExecRatio(std::stod(configServices.find("REQ")->second.at(1)));
    serv_REQ->setOutputFunction(return_REQ);
    serv_REQ->setExecAmountFunc(pr_REQ);
    serv_REQ->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_REQ->addHost(simgrid::s4u::Host::by_name(configServices.find("REQ")->second.at(0)));
    simgrid::s4u::Actor::create("etm_REQ", simgrid::s4u::Host::by_name(configServices.find("REQ")->second.at(0)), [serv_REQ] { serv_REQ->run(); });


    // create ETM for service servA
    std::vector<std::string> v_serv_servA = std::vector<std::string>();
    v_serv_servA.push_back("servA");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servA = std::make_shared<sg_microserv::ElasticTaskManager>("servA",v_serv_servA);
    serv_servA->setBootDuration(0);
    // serv_servA->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servA->setParallelTasksPerInst(std::stoi(configServices.find("servA")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servA->setExecRatio(std::stod(configServices.find("servA")->second.at(1)));
    serv_servA->setOutputFunction(return_servA);
    serv_servA->setExecAmountFunc(pr_servA);
    serv_servA->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servA->addHost(simgrid::s4u::Host::by_name(configServices.find("servA")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servA", simgrid::s4u::Host::by_name(configServices.find("servA")->second.at(0)), [serv_servA] { serv_servA->run(); });


    // create ETM for service servB
    std::vector<std::string> v_serv_servB = std::vector<std::string>();
    v_serv_servB.push_back("servB");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servB = std::make_shared<sg_microserv::ElasticTaskManager>("servB",v_serv_servB);
    serv_servB->setBootDuration(0);
    // serv_servB->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servB->setParallelTasksPerInst(std::stoi(configServices.find("servB")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servB->setExecRatio(std::stod(configServices.find("servB")->second.at(1)));
    serv_servB->setOutputFunction(return_servB);
    serv_servB->setExecAmountFunc(pr_servB);
    serv_servB->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servB->addHost(simgrid::s4u::Host::by_name(configServices.find("servB")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servB", simgrid::s4u::Host::by_name(configServices.find("servB")->second.at(0)), [serv_servB] { serv_servB->run(); });


    // create ETM for service servD
    std::vector<std::string> v_serv_servD = std::vector<std::string>();
    v_serv_servD.push_back("servD");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servD = std::make_shared<sg_microserv::ElasticTaskManager>("servD",v_serv_servD);
    serv_servD->setBootDuration(0);
    // serv_servD->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servD->setParallelTasksPerInst(std::stoi(configServices.find("servD")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servD->setExecRatio(std::stod(configServices.find("servD")->second.at(1)));
    serv_servD->setOutputFunction(return_servD);
    serv_servD->setExecAmountFunc(pr_servD);
    serv_servD->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servD->addHost(simgrid::s4u::Host::by_name(configServices.find("servD")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servD", simgrid::s4u::Host::by_name(configServices.find("servD")->second.at(0)), [serv_servD] { serv_servD->run(); });


    // create ETM for service servC
    std::vector<std::string> v_serv_servC = std::vector<std::string>();
    v_serv_servC.push_back("servC");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servC = std::make_shared<sg_microserv::ElasticTaskManager>("servC",v_serv_servC);
    serv_servC->setBootDuration(0);
    // serv_servC->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servC->setParallelTasksPerInst(std::stoi(configServices.find("servC")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servC->setExecRatio(std::stod(configServices.find("servC")->second.at(1)));
    serv_servC->setOutputFunction(return_servC);
    serv_servC->setExecAmountFunc(pr_servC);
    serv_servC->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servC->addHost(simgrid::s4u::Host::by_name(configServices.find("servC")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servC", simgrid::s4u::Host::by_name(configServices.find("servC")->second.at(0)), [serv_servC] { serv_servC->run(); });


    // create ETM for service servA2
    std::vector<std::string> v_serv_servA2 = std::vector<std::string>();
    v_serv_servA2.push_back("servA2");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servA2 = std::make_shared<sg_microserv::ElasticTaskManager>("servA2",v_serv_servA2);
    serv_servA2->setBootDuration(0);
    // serv_servA2->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servA2->setParallelTasksPerInst(std::stoi(configServices.find("servA2")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servA2->setExecRatio(std::stod(configServices.find("servA2")->second.at(1)));
    serv_servA2->setOutputFunction(return_servA2);
    serv_servA2->setExecAmountFunc(pr_servA2);
    serv_servA2->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servA2->addHost(simgrid::s4u::Host::by_name(configServices.find("servA2")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servA2", simgrid::s4u::Host::by_name(configServices.find("servA2")->second.at(0)), [serv_servA2] { serv_servA2->run(); });


    // create ETM for service servC2
    std::vector<std::string> v_serv_servC2 = std::vector<std::string>();
    v_serv_servC2.push_back("servC2");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servC2 = std::make_shared<sg_microserv::ElasticTaskManager>("servC2",v_serv_servC2);
    serv_servC2->setBootDuration(0);
    // serv_servC2->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servC2->setParallelTasksPerInst(std::stoi(configServices.find("servC2")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servC2->setExecRatio(std::stod(configServices.find("servC2")->second.at(1)));
    serv_servC2->setOutputFunction(return_servC2);
    serv_servC2->setExecAmountFunc(pr_servC2);
    serv_servC2->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servC2->addHost(simgrid::s4u::Host::by_name(configServices.find("servC2")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servC2", simgrid::s4u::Host::by_name(configServices.find("servC2")->second.at(0)), [serv_servC2] { serv_servC2->run(); });


    // create ETM for service servD2
    std::vector<std::string> v_serv_servD2 = std::vector<std::string>();
    v_serv_servD2.push_back("servD2");
    std::shared_ptr<sg_microserv::ElasticTaskManager> serv_servD2 = std::make_shared<sg_microserv::ElasticTaskManager>("servD2",v_serv_servD2);
    serv_servD2->setBootDuration(0);
    // serv_servD2->setDataSizeRatio(1); NOT USED ANYMORE, KEEP AS COMMENT IN CASE
    // modify this value in the configuration file (parDeg)
    serv_servD2->setParallelTasksPerInst(std::stoi(configServices.find("servD2")->second.at(2)));
    // modify this value in the  configuration file (execRatio)
    serv_servD2->setExecRatio(std::stod(configServices.find("servD2")->second.at(1)));
    serv_servD2->setOutputFunction(return_servD2);
    serv_servD2->setExecAmountFunc(pr_servD2);
    serv_servD2->setReqNames(reqTypeToStr);
    // set the host location in the config file (location)
    serv_servD2->addHost(simgrid::s4u::Host::by_name(configServices.find("servD2")->second.at(0)));
    simgrid::s4u::Actor::create("etm_servD2", simgrid::s4u::Host::by_name(configServices.find("servD2")->second.at(0)), [serv_servD2] { serv_servD2->run(); });

  /* ADD DATASOURCES MANUALLY HERE, SET THE END TIMER AS YOU WISH, AND LAUNCH YOUR SIMULATOR*/
  simgrid::s4u::this_actor::sleep_for(1);
std::vector<simgrid::s4u::CommPtr> pending_comms;
  simgrid::s4u::Mailbox* mb = s4u_Mailbox::by_name("REQ");
  boost::uuids::random_generator generator;
  for (int i = 0 ; i < freq; i++){
    double date = simgrid::s4u::Engine::get_instance()->get_clock();
    sg_microserv::TaskDescription* t = new sg_microserv::TaskDescription(generator(), date);
    t->firstArrivalDate = date;

    t->dSize = 100;
    t->requestType = RequestType::LOGIN;

    simgrid::s4u::CommPtr comm = mb->put_async(t, t->dSize);
pending_comms.push_back(comm);
  }


simgrid::s4u::this_actor::sleep_for(1);

  // kill policies and ETMs
  //simgrid::s4u::this_actor::sleep_for(10); /*set it according to your needs*/
  XBT_INFO("Done. Killing policies and etms");
    serv_REQ->kill();
    serv_servA->kill();
    serv_servB->kill();
    serv_servD->kill();
    serv_servC->kill();
    serv_servA2->kill();
    serv_servC2->kill();
    serv_servD2->kill();
}
int main(int argc, char* argv[]) {
  if (argc <= 2) {
    std::cout << "Wrong execution line" << std::endl;
		std::cout << argv[0] << "<simgrid-platform-file.xml> <services-config-file.csv>" << std::endl;
		exit(1);
	}

  // read the config from the file provided by the user
  std::ifstream input(argv[2]);
  std::map<std::string, std::vector<std::string>> servConfig;
  std::string v;
  // pass header
  input>>v>>v>>v>>v;
  for (std::string servName, servLocation, servExecRatio, servParDeg; input >> servName >> servLocation >> servExecRatio >> servParDeg;) {
        servConfig[servName] = {servLocation, servExecRatio, servParDeg};
  }

	simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);

	e->load_platform(argv[1]);
	simgrid::s4u::Actor::create("main", simgrid::s4u::Host::by_name(servConfig.find("default")->second.at(0)), [&] {run(servConfig, std::stod(argv[3]));});
	e->run();
	return 0;
}
