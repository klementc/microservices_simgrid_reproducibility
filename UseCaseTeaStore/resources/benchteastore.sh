duration=30

csv=teastore.csv
echo "nbc,load,reqpersec">${csv}

set -x
for nbc in 24
do
	for load in `seq 1 20 2600`
	do
		file=teastore_${nbc}_${load}.log
		./teastore_login ../../config_files/platforms/two_hosts_100p_${nbc}c.xml ../../config_files/configServices/teaStore_LOGIN_config.csv ${load} > $file 2>&1
		reqPerSec=`awk  -f teastore.awk $file`
		echo "${nbc},${load},${reqPerSec}" >> ${csv}
	done
done
