#!/usr/bin/awk -f

BEGIN {
	amount=0
}

/FINISHED REQUEST/{
	amount=amount+1
}

END {
	print(amount)
}
