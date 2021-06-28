out="resTeaStoreCombined.csv"

echo "Sample,nbC,Target Time,Load Intensity,Successful Transactions,Failed Transactions,Dropped Transactions,Avg Response Time,Final Batch Dispatch Time,18.05.2021;12:07:17444
" > ${out}

# number of samples
for nbS in `seq 1 1 20`
do
  # core configurations
  for nbC in 4 8 16
  do
    awk -v prefix1="$nbS" -v prefix2="$nbC" 'NR > 1{print prefix1","prefix2","$0}' res_${nbC}c_${nbS}.csv >> ${out}
  done
done
