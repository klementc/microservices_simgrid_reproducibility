suff="${1}"
N1COST="1100000"
#for i in `seq 1 1 3`
#do
    docker run  --ulimit nofile=120000 -d -v ~/log_dock:/logs --cpus=1.0 --cpuset-cpus=4 -e cpuload=100 --network host --rm -ti expe/rmqexpe:latest /bin/bash -c "cd /expeFiles && logDir=/logs suffix=exp1_${suff} N1COST=${N1COST} bash launchOnce.sh"
    docker run  --ulimit nofile=120000 -d -v ~/log_dock:/logs --cpus=1.0 --cpuset-cpus=5 -e cpuload=100 --network host --rm -ti expe/rmqexpe:latest /bin/bash -c "cd /expeFiles && logDir=/logs suffix=exp2_${suff} N1COST=${N1COST} bash launchOnce.sh"
    docker run  --ulimit nofile=120000 -d -v ~/log_dock:/logs --cpus=1.0 --cpuset-cpus=6 -e cpuload=100 --network host --rm -ti expe/rmqexpe:latest /bin/bash -c "cd /expeFiles && logDir=/logs suffix=exp3_${suff} N1COST=${N1COST} bash launchOnce.sh"
#done
