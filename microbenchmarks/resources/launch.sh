#!/bin/bash

# exec run
[ -z "${resF}" ] && resF="results"
[ -z "${logDir}" ] && logDir="./execLogs"
[ -z "${start}" ] && start="20000000"
[ -z "${end}" ] && end="50000000"
[ -z "${incr}" ] && incr="2000000"
[ -z "${tsFile}" ] && tsFile="default5TimeStamps.csv"
[ -z "${parDeg}" ] && parDeg=10

if [[ ! -e ${logDir} ]]
then
    mkdir -p ${logDir}
fi

if [[ -z ${scenario} ]]
then
    echo "You need to specify the scenario you want to launch (1-4)"
    exit 1
fi

echo "ts,qArr,instArr,startEx,endEx,flops,serv" > "${logDir}/${resF}.csv"

for s in `seq ${start} ${incr} ${end}`
do
    suffix="1_${s}"
    echo "ts,qArr,instArr,startEx,endEx,flops,serv" > "${logDir}/${resF}_${s}.csv"
    echo "$(pwd)/dejavu_platform.xml"
    echo "-------------------------------"
    echo "starting experiment loops: ${s}, scenario: ${scenario}"

    if [[ "1" = ${scenario} ]]
    then
        ./bench1  "$(pwd)/dejavu_platform.xml" ${s} ${tsFile} ${parDeg} > ${logDir}/sg_${s}.log 2>&1
        echo "done"
    elif [[ "2" = ${scenario} ]]
    then
        ./bench2  "$(pwd)/dejavu_platform.xml" ${s} ${s} ${tsFile} ${parDeg} > ${logDir}/sg_${s}.log 2>&1
        echo "done"
    elif [[ "3" = ${scenario} ]]
    then
        if [[ -z ${fixedCost} ]]
        then
            echo "Error, you need to set \${fixedCost} to execute scenario ${scenario}"
            exit 1
        fi
        ./bench2  "$(pwd)/dejavu_platform.xml" ${fixedCost} ${s} ${tsFile} ${parDeg} > ${logDir}/sg_${s}.log 2>&1
        echo "done"
    elif [[ "4" = ${scenario} ]]
    then
        if [[ -z ${fixedCost} ]]
        then
            echo "Error, you need to set \${fixedCost} to execute scenario ${scenario}"
            exit 1
        fi
        ./bench2  "$(pwd)/dejavu_platform.xml" ${s} ${fixedCost} ${tsFile} ${parDeg} > ${logDir}/sg_${s}.log 2>&1
        echo "done"
    else
        echo "Unknown scenario ${scenario}"
    fi

    awk -f parse.awk "${logDir}/sg_${s}.log" > "${logDir}/${resF}_${s}.csv"

    tail -n+1 "${logDir}/${resF}_${s}.csv" >> "${logDir}/${resF}.csv"

done
