nbC=16
nbThreads=64

for sample in `seq 15 1 20`
do
	java -jar httploadgenerator.jar director -a examples/httploadgenerator/increasingHighIntensity.csv -l examples/httploadgenerator/teastore_browse_2.lua -o res_${nbC}c_${sample}.csv -t ${nbThreads}
	sleep 30
done
