#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <simgrid/s4u/Mailbox.hpp>
#include <simgrid/s4u/Comm.hpp>
#include <simgrid/s4u/Engine.hpp>

#include "DataSource.hpp"
#include "RequestType.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(dataSource, "logs for the Graph experiment");

namespace sg_microserv {

void DataSource::run() {
  std::vector<simgrid::s4u::CommPtr> pending_comms;
  boost::uuids::random_generator generator;
  simgrid::s4u::Mailbox* mb = s4u_Mailbox::by_name(mbName_);

  while (keepGoing_) {
    // clean finished puts
    for (std::vector<simgrid::s4u::CommPtr>::iterator it = pending_comms.begin()
      ; it != pending_comms.end();) {
        if ((*it)->test())
          it = pending_comms.erase(it);
        else
          it++;
    }

    // for a request you need to know:
    //  -> the timestamps at which it is triggered
    //  -> the size of the request sent over the network
    double evtTS = getNextReqTS();
    double nextSize = getNextReqSize();

    if (evtTS == -1) {simgrid::s4u::Comm::wait_all(&pending_comms); return;}

    simgrid::s4u::this_actor::sleep_until(evtTS);
    double date = simgrid::s4u::Engine::get_instance()->get_clock();
    TaskDescription* t = new TaskDescription(generator(), date);
    t->firstArrivalDate = date;
    XBT_DEBUG("send %p", t);

    t->dSize = nextSize;
    t->requestType = rt_;

    simgrid::s4u::CommPtr comm = mb->put_async(t, t->dSize);
    pending_comms.push_back(comm);
  }
}

void DataSource::suspend() {
  xbt_assert(keepGoing_, "Cannot suspend, datasource already stopepd");
  keepGoing_ = false;
}

/* FixedInterval example */
double DataSourceFixedInterval::getNextReqTS() {
  // next trigger is equal to the previous one plus the interval
  previousTrigger_ += interval_;
  return previousTrigger_;
}

double DataSourceFixedInterval::getNextReqSize() {
  return reqSize_;
}

/* Timestamps File example */
double DataSourceTSFile::getNextReqTS() {
  // next trigger is equal to the next line of the file
  double ts;
  file_ >> ts;

  if (file_.eof()) return -1;

  return ts;
}

double DataSourceTSFile::getNextReqSize() {
  return reqSize_;
}

}  // namespace sg_microserv
