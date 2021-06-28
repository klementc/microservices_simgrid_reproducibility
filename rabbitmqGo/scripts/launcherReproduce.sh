#!/bin/bash

function printParams {
  echo "----------------------------------------------------"
  echo "PARAMETERS:"
  echo "suffix: ${suffix}"
  echo "Parallelization degree: ${parD}"
  echo "HostLogPath: ${hostLogPath}"
  echo "nbIters: start=${start}, end=${end}, iter=${iter}"
  echo "Samples: ${nbSamples}"
  echo "Timestamp file: ${tsFile}"
  echo "First core used: ${firstCore}"
  echo "----------------------------------------------------"
}

DATE=`date +%Y-%m-%d_%H-%M-%S`
[ ! -z "${figlet}" ] && figlet "Mserv reprod"
echo "Timestamp: ${DATE}"

if [[ -z "${scenario}" ]]
then
  echo "No scenario specified, please set \${scenario} to the scenario number you wish to run"
  exit 1
fi

###############################################################################
if [[ ! -z ${onG5K} ]]
then
  echo "Running on G5K, proceeding to the setup"
  if [[ -z "${repoPath}" ]]
  then
    echo "Please specify your path to the G5K reproductibility git in \${repoPath}"
    exit 1
  fi

  echo "Setup docker-g5k.."
  g5k-setup-docker -t
  echo "Install go using sudo-g5k"
  sudo-g5k apt-get -y install golang
  echo "Move to the directory.."
  cd ${repoPath}
  echo "Build go services.."
  go build cmd/sinkService/sinkService.go
  go build cmd/dataSourceService/senderService.go
  go build cmd/computeService/computeService.go
  echo "Build the docker container from Dockerfile"
  docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t expe/rmqgo .
  echo "Done! Going to script/ to run the experiment"
else
  echo "Running locally..."
fi


[ -z "${run}" ] && echo "Set \${run} to launch the experiment in addition to the setup" && exit 1

cd ${repoPath}/scripts/
###############################################################################
[ ! -z "${figlet}" ] && figlet "First step: calib"
echo "Before running the actual experiment, let s do a calibration run with 10 points (a bit less than an hour)"
firstCore="0"
parD="10"
suffix="reprodcalib"
hostLogPath="${HOME}/logs_expe/goLogs/reprod_calib_${DATE}/"
start="1"
end="100000002"
iter="10000000"
nbSamples="4"
tsFile="/go/src/app/timestamps/tsCal.csv"
printParams
mkdir -p ${hostLogPath}
scenario=${scenario} parD=${parD} suffix=${suffix} hostLogPath=${hostLogPath} logDir=/logs/ tsFile=${tsFile} start=${start} end=${end} iter=${iter} nbSamples=${nbSamples} durIter=360 bash launchPar.sh 2>&1 > ${hostLogPath}/launchPar_logs.log

###############################################################################
[ ! -z "${figlet}" ] && figlet "Second step: run"

if [[ ${scenario} = "1" ]]
then
  echo "Running scenario 1"
  firstCore="0"
  parD="10"
  suffix="reprodscen1"
  hostLogPath="${HOME}/logs_expe/goLogs/reprod_scenario1_${DATE}/"
  start="1"
  end="80000001"
  iter="2000000"
  nbSamples="4"
  tsFile="/go/src/app/timestamps/default5TimeStamps.csv"
  printParams
  mkdir -p ${hostLogPath}
  scenario=1 parD=${parD} suffix=${suffix} hostLogPath=${hostLogPath} logDir=/logs/ tsFile=${tsFile} start=${start} end=${end} iter=${iter} nbSamples=${nbSamples} durIter=360 bash launchPar.sh 2>&1 > ${hostLogPath}/launchPar_logs.log
elif [[ ${scenario} = "2" ]]
then
  echo "Running scenario 2"
  firstCore="0"
  parD="10"
  suffix="reprodscen2"
  hostLogPath="${HOME}/logs_expe/goLogs/reprod_scenario2_${DATE}/"
  start="1"
  end="80000001"
  iter="2000000"
  nbSamples="2"
  tsFile="/go/src/app/timestamps/default5TimeStamps.csv"
  printParams
  mkdir -p ${hostLogPath}
  scenario=2 parD=${parD} suffix=${suffix} hostLogPath=${hostLogPath} logDir=/logs/ tsFile=${tsFile} start=${start} end=${end} iter=${iter} nbSamples=${nbSamples} durIter=360 bash launchPar.sh 2>&1 > ${hostLogPath}/launchPar_logs.log
elif [[ ${scenario} = "3" ]]
then
  echo "Running scenario 3"
  firstCore="0"
  parD="10"
  suffix="reprodscen3"
  hostLogPath="${HOME}/logs_expe/goLogs/reprod_scenario3_${DATE}/"
  start="1"
  end="80000001"
  iter="2000000"
  nbSamples="4"
  tsFile="/go/src/app/timestamps/default5TimeStamps.csv"
  printParams
  mkdir -p ${hostLogPath}
  scenario=3 parD=${parD} suffix=${suffix} hostLogPath=${hostLogPath} logDir=/logs/ tsFile=${tsFile} start=${start} end=${end} iter=${iter} nbSamples=${nbSamples} durIter=360 bash launchPar.sh 2>&1 > ${hostLogPath}/launchPar_logs.log
elif [[ ${scenario} = "4" ]]
then
  echo "Running scenario 4"
  firstCore="0"
  parD="10"
  suffix="reprodscen4"
  hostLogPath="${HOME}/logs_expe/goLogs/reprod_scenario4_${DATE}/"
  start="1"
  end="80000001"
  iter="2000000"
  nbSamples="4"
  tsFile="/go/src/app/timestamps/default5TimeStamps.csv"
  printParams
  mkdir -p ${hostLogPath}
  scenario=4 parD=${parD} suffix=${suffix} hostLogPath=${hostLogPath} logDir=/logs/ tsFile=${tsFile} start=${start} end=${end} iter=${iter} nbSamples=${nbSamples} durIter=360 bash launchPar.sh 2>&1 > ${hostLogPath}/launchPar_logs.log
else
  echo "Unknown scenario ${scenario}, please specify an existing scenario"
fi