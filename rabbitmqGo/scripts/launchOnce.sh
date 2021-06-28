#!/bin/bash

# launch locally (1 node) services of bench infrastructures
[ -z "${N1COST}" ] && N1COST="10000"
[ -z "${N2COST}" ] && N2COST="10000"
[ -z "${logDir}" ] && logDir="/logs/"
[ -z "${hostLogPath}" ] && hostLogPath="${HOME}/logs_expe/goLogs/"
[ -z "${suffix}" ] && suffix="DEFAULT"
[ -z "${rootCode}" ] && rootCode="../"
[ -z "${tsFile}" ] && tsFile="/go/src/app/timestamps/tsCal.csv"
[ -z "${parD}" ] && parD="25"
[ -z "${hostMQ}" ] && hostMQ="amqp://guest:guest@localhost:5672/"
[ -z "${firstCore}" ] && firstCore=0

if [[ -z ${scenario} ]]
then
    echo "Indicate the scenario to launch for launchOnce.sh to run"
    exit 1
fi

if [[ -z ${hostLogPath} ]]
then
    mkdir -p ${hostLogPath}
fi


# IMPORTANT: remove previous services
sleep 3
echo "Launch infrastructure with scenario ${scenario}"
firstCore=${firstCore} hostLogPath=${hostLogPath} parD=${parD} hostMQ=${hostMQ} suffix=${suffix} logDir=${logDir} N1COST=${N1COST} N2COST=${N2COST} scenario=${scenario} bash deployInfra.sh
sleep 10
echo "Launch datasource"
docker run -v ${hostLogPath}:/logs --network host --rm expe/rmqgo:latest /bin/bash -c "/go/src/app/senderService -s scenario${scenario} -r ${hostMQ} -o serv1 -t ${tsFile} 2>&1 | tee ${logDir}/${scenario}_DS_${N1COST}_${suffix}.log"

echo "Datasource sent all of its messages, parse resulting logs for scenario ${scenario}"
if [[ ${scenario} = "1" ]]
then
    echo "See ${hostLogPath}/results_1_${N1COST}_${suffix}.csv"
    awk -f parse.awk "${hostLogPath}/1_S1_${N1COST}_${suffix}.log" > "${hostLogPath}/results_1_${N1COST}_${suffix}.csv"
elif [[ ${scenario} = "2" || ${scenario} = "3" || ${scenario} = "4" ]]
then
    echo "See ${hostLogPath}/results_${scenario}_${N1COST}_${N2COST}_${suffix}.csv"
    awk -f parse.awk "${hostLogPath}/${scenario}_S1_${N1COST}_${suffix}.log" > "${hostLogPath}/results_${scenario}_${N1COST}_${N2COST}_${suffix}.csv"
    awk -f parse.awk "${hostLogPath}/${scenario}_S2_${N2COST}_${suffix}.log" | tail -n+2 >> "${hostLogPath}/results_${scenario}_${N1COST}_${N2COST}_${suffix}.csv"
else
    echo "UNKNOWN SCENARIO number ${scenario}, cannot parse"
fi