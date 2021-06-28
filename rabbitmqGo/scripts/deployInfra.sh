#!/bin/bash

[ -z "${N1COST}" ] && N1COST="10000000"
[ -z "${N2COST}" ] && N2COST="10000000"
[ -z "${hostLogPath}" ] && hostLogPath="${HOME}/logs_expe/goLogs/"
[ -z "${logDir}" ] && logDir="/logs/"
[ -z "${suffix}" ] && suffix="DEFAULT"
[ -z "${parD}" ] && parD="25"

# kill already launched application
if [[ $1 = "kill" ]]
then
    pkill -9 -f computeService.go
    pkill -9 -f sinkService.go
    pkill -9 computeService
    pkill -9 sinkService
    exit
fi

if [[ -z "${hostMQ}" ]]
then
    echo "No hostMQ provided, terminate (deployinfra.sh)"
    exit 1
fi

if [[ -z ${firstCore} ]]
then
    echo "First core to run containers required (deployinfra.sh)"
    exit 1
fi

if [[ -z ${scenario} ]]
then
    echo "Scenario required (deployinfra.sh)"
    exit 1
fi

if [[ ! -e ${hostLogPath} ]]
then
mkdir -p ${hostLogPath}
fi

# launch the most simple application with only 1 service
if [[ ${scenario} = "1" ]]
then

    echo "Configuration 1: DS -> S1 -> Sink (launch DS to queue 'serv1' to start)"
    # compute service
    echo "Launch S1 on core ${firstCore}"
    docker run -d -v ${hostLogPath}:/logs --cpus=1.0 --cpuset-cpus=${firstCore} -e cpuload=100 --network host --rm -ti expe/rmqgo:latest /bin/bash -c "/go/src/app/computeService -s scenario1 -r ${hostMQ} -i serv1 -o sink -n S1_${suffix} -c ${N1COST} -p ${parD} 2>&1 | tee ${logDir}/1_S1_${N1COST}_${suffix}.log"

    # sink service
    echo "Launch Sink on core $((${firstCore}+1))"
    docker run -d -v ${hostLogPath}:/logs --cpus=1.0 --cpuset-cpus=$((${firstCore}+1)) -e cpuload=100 --network host --rm -ti expe/rmqgo:latest /bin/bash -c "/go/src/app/sinkService -s scenario1 -r ${hostMQ} -i sink -n sinkServ 2>&1 | tee ${logDir}/1_Sink_${N1COST}_${suffix}.log"

    echo "Application ready to start. Create a dataSource on queue serv1"
# second infrastructure with 2 services in a row
elif [[ ${scenario} = "2" || ${scenario} = "3" || ${scenario} = "4" ]]
then
    echo "Configuration ${scenario}: DS -> S1 -> S2 -> Sink (launch DS to queue 'serv1' to start)"
    # First compute service (S1)
    docker run -d -v ${hostLogPath}:/logs --cpus=1.0 --cpuset-cpus=${firstCore} -e cpuload=100 --network host --rm -ti expe/rmqgo:latest /bin/bash -c "/go/src/app/computeService -s scenario${scenario} -r ${hostMQ} -i serv1 -o serv2 -n S1_${suffix} -c ${N1COST} -p ${parD} 2>&1 | tee ${logDir}/${scenario}_S1_${N1COST}_${suffix}.log"

    # Second compute service (S2)
    docker run -d -v${hostLogPath}:/logs --cpus=1.0 --cpuset-cpus=$((${firstCore}+1)) -e cpuload=100 --network host --rm -ti expe/rmqgo:latest /bin/bash -c "/go/src/app/computeService -s scenario${scenario} -r ${hostMQ} -i serv2 -o sink  -n S2_${suffix} -c ${N2COST} -p ${parD} 2>&1 | tee ${logDir}/${scenario}_S2_${N2COST}_${suffix}.log"

    # sink service
    docker run -d -v ${hostLogPath}:/logs --cpus=1.0 --cpuset-cpus=$((${firstCore}+2)) -e cpuload=100 --network host --rm -ti expe/rmqgo:latest /bin/bash -c "/go/src/app/sinkService -s scenario${scenario} -r ${hostMQ} -i sink -n sink 2>&1 | tee ${logDir}/${scenario}_Sink_${N1COST}_${N2COST}_${suffix}.log"
    echo "Application ready to start. Create a dataSource on queue serv1"

# add elif cases for future infrastructures
fi
