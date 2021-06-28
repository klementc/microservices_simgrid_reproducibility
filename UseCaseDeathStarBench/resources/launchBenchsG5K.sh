
set -x

duration=30

scriptDir="${PWD}"
execDir="/root/DeathStarBench/socialNetwork/"
csv=res.csv
resDir="/root/DeathStarBench/socialNetwork/res2705/"

if [[ -z "${perc}" ]]
then
    echo "don't start, set perc"
    exit 1
fi

nbC=10

[[ ! -e "${resDir}/resTot.csv" ]] && echo "perc,nbn,sample,conn,load,meanLat,reqPerSec,nbC">"${resDir}/resTot.csv"

    for nbn in 2
    do
	for nbSample in `seq 7 1 8`
	do
	    outDir="${resDir}/res_${perc}p/s${nbSample}_n${nbn}_c${nbC}"
	    mkdir -p ${outDir}
	    echo "perc,nbn,sample,conn,load,meanLat,reqPerSec">"${outDir}/${csv}"

	    for load in `seq 1600 100 2650`
	    do
		conn=$(( ${load} / 100 ))

		cd ${execDir}
#		docker-compose -f docker-compose.yml up -d
		docker stack deploy --compose-file docker_compose_${nbn}_launchable.yml testDSB
		sleep 30
		cd ${execDir}/wrk2/

		file=out_${load}_${nbSample}.log
		./wrk -D exp -t 1 -c ${conn} -d ${duration}s -L -s ./scripts/social-network/compose-post.lua http://localhost:8080/wrk2-api/post/compose -R ${load} -P > "${outDir}/${file}"
		echo "lat" > ${outDir}/latPerReq_${load}_${nbSample}.csv
		mv 0.txt ${outDir}/latPerReq_${load}_${nbSample}.csv
		meanL=`awk  '/, StdDeviation/ { printf "%s", $3 ; }' ${outDir}/${file}`
		reqPerSec=`awk  '/Requests[/]sec:/ { printf "%s", $2 ; }' ${outDir}/${file}`
		echo "${perc},${nbn},${nbSample},${conn},${load},${meanL}${reqPerSec},${nbC}" >> "${outDir}/${csv}"
		echo "${perc},${nbn},${nbSample},${conn},${load},${meanL}${reqPerSec},${nbC}" >> "${resDir}/resTot.csv"

		curl -s 'http://localhost:16686/api/traces?service=testDSB_nginx-web-server&lookback=20m&prettyPrint=true&limit=1500' > "${outDir}/traces.json"
		cd ${execDir}
		docker stack rm testDSB
		sleep 60
#		docker-compose down
		docker system prune --volumes -f
	    done
	done
    done
