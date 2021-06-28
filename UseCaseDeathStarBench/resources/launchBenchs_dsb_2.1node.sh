
duration=30

csv=res_dsb_2n_2805.csv
csvlat=reslat_dsb_2n_2805.csv
#echo "ratio,nbn,perc,load,lat">${csvlat}
#echo "nbn,load,reqPerSec">${csv}

set -x
for load in `seq 1 100 2002`
do
		for nbn in 2.1
		do
		file=out_${nbn}_${load}.log
#		./dsb_compose ../../platforms/two_hosts_${perc}p.xml config${nbn}.csv ${load} > $file 2>&1
#./generated ../../config_files/platforms/two_hosts_100p_10c.xml ../../config_files/configServices/config${nbn}node_100.csv ${load} > $file 2>&1
		./generated_2inst ../../config_files/platforms/two_hosts_100p_10c.xml ../../config_files/configServices/config${nbn}node_100.csv ${load} > $file 2>&1
		reqPerSec=`awk  -f dsb2.awk $file`
		echo "${nbn},${load},${reqPerSec}" >> ${csv}
		awk -f dsb.awk $file > ${file}.csv
		awk -v va="100" -v vb="$nbn" -v vc="100" -v vd="$load" -v ve="$reqPerSec" -f dsblat.awk $file >> $csvlat
		done
done
