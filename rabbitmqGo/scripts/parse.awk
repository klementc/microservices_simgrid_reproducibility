#!/usr/bin/awk -f

BEGIN {
    print ("serviceName,nbIter,timestamp,totDurInServ,computeDur,parExec,totReqProcessed,scenario")
}

# fetch first timestamp and remove it later from all other to start from time 0
NR == 2 {
    DELTA = $6
#    print("substract delta from ts:"$9)
}

/EndReqLog/{
    # format: serviceName nbIter timestamp totDurInServ computeDur parExec
    print($4","$5","$6 - DELTA","$7","$8","$9","$10","$11)
}

# TODO: parse for other services than compute

END {
}
