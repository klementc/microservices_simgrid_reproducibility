#ifndef ELASTICITY_DATASOURCE_HPP_
#define ELASTICITY_DATASOURCE_HPP_

#include <vector>
#include <string>

#include "ElasticConfig.hpp"
#include "ElasticTask.hpp"
#include "RequestType.hpp"

namespace sg_microserv {

class DataSource {
 private:
    /**
     * The name of the mailbox to which requests will be sent
     */
    std::string mbName_;
    bool keepGoing_;
    RequestType rt_;
    /**
     * Returns the next timestamps at which the node should send a request
     * implement this method with your own data production scheme
     */
    virtual double getNextReqTS() = 0;

    // virtual double getNextFlopAmount() = 0;
    virtual double getNextReqSize() = 0;

 public:
    DataSource(std::string mbName, RequestType rt)
      : mbName_(mbName), keepGoing_(true), rt_(rt) {}

    explicit DataSource(std::string mbName)
      : DataSource(mbName_, RequestType::DEFAULT) {}
    void run();
    void suspend();
};

/**
 * Example of a custom dataSource
 *  -> Requests are triggered at a fixed interval
 *  -> Requests size is fixed
 */
class DataSourceFixedInterval : public DataSource {
 private:
    double interval_;
    double reqSize_;
    double previousTrigger_;
 public:
    DataSourceFixedInterval(std::string mbName, RequestType rt, double interval, double reqS)
      :DataSource(mbName, rt), interval_(interval), previousTrigger_(0), reqSize_(reqS) {}
    DataSourceFixedInterval(std::string mbName, double interval, double reqS)
      :DataSourceFixedInterval(mbName, RequestType::DEFAULT, interval, reqS) {}
    virtual double getNextReqSize();
    virtual double getNextReqTS();
};

/**
 * Example of a custom dataSource
 *  -> Requests are triggered given a timestamp file
 *  -> Fixed requests size
 * The file should containe one timestamp per line (first line ignored to allow for csv headers)
 */
class DataSourceTSFile : public DataSource {
 private:
    std::ifstream file_;
    double reqSize_;
 public:
    DataSourceTSFile(std::string mbName, RequestType rt, std::string fName, double reqSize)
      :DataSource(mbName, rt), reqSize_(reqSize) {file_.open(fName); std::string a; file_ >> a;}
    DataSourceTSFile(std::string mbName, std::string fName, double reqSize)
      :DataSourceTSFile(mbName, RequestType::DEFAULT, fName, reqSize) {}
    virtual double getNextReqSize();
    virtual double getNextReqTS();
};

}  // namespace sg_microserv

#endif  // ELASTICITY_DATASOURCE_HPP_
