#!/bin/bash

# launch locally (1 node) services of bench infrastructures
[ -z "${N1COST}" ] && N1COST="10000000"
[ -z "${logDir}" ] && logDir="./logs/"
[ -z "${suffix}" ] && suffix="DEFAULT"
[ -z "${rootCode}" ] && rootCode="../"
[ -z "${tsFile}" ] && tsFile="../../rabbitmqService/tsCal.csv"
[ -z "${parD}" ] && parD="25"

[ -z "${start}" ] && start=1000000
[ -z "${iter}" ] && iter=5000000
[ -z "${end}" ] && end=10000000
[ -z "${nbSamples}" ] && nbSamples=3

if [[ ! -e ${logDir} ]]
then
    mkdir -p ${logDir}
fi

echo "Launching calibration:"
echo " ${nbSamples} executions per value"
echo " value start: ${start} end: ${end} incr: ${iter}"

#header of aggregated results file
echo "serviceName,nbIter,timestamp,totDurInServ,computeDur,parExec,totReqProcessed" > "${logDir}/aggrResults_${start}-${iter}-${end}_${nbSamples}.csv"

for s in `seq ${start} ${iter} ${end}`
do
    echo "--------------------"
    echo "new Loop value: ${s}"
    for sample in `seq 1 ${nbSamples}`
    do
	suffix="1_s${s}_sample${sample}"
	bash deployInfra.sh kill
	echo "-------------------------------"
	echo "starting experiment loops: ${s}, sample: ${sample}"	
	echo "Build Infrastructure"
	parD=${parD} N1COST="${s}" logDir="${logDir}" suffix="${suffix}" bash deployInfra.sh 1
	sleep 1
	echo "Start data Source"
	parD=${parD} N1COST="${s}" logDir="${logDir}" suffix="${suffix}" tsFile="${tsFile}" bash launchOnce.sh 
	
	sleep 10	
	echo "-------------------------------"
	echo "finished run, append to results_aggr_${suffix}.csv"
	echo "TODO PARSE RESULTS OF SINK AND SOURCE"
	# compute service

	tail -n+2 "${logDir}/results_${s}_${suffix}.csv" >> "${logDir}/aggrResults_${start}-${iter}-${end}_${nbSamples}.csv"
    done   
done
