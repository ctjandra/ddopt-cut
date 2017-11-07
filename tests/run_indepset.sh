#!/bin/bash

# Very simple script to run experiments in parallel

if [ -z "$1" ]; then
	echo "Error: No argument supplied (number of jobs); argument must be between 1 and 10 for independent set"
	echo "Recommended 5 or 10 for independent set and 8 or 16 for set cover (if enough cores/processors are available)"
	exit 1
fi

if [ $1 -gt 10 -o $1 -lt 1 ]; then
	echo "Error: Argument must be between 1 and 10 for independent set"
	echo "Recommended 5 or 10 for independent set and 8 or 16 for set cover (if enough cores/processors are available)"
	exit 1
fi

if [ ! -e "bin/ddopt" ]; then
	echo "Error: Binary file ddopt does not exist in bin directory; make sure you have compiled the program"
	exit 1
fi

if [ ! -d "instances/indepset" ]; then
	echo "Error: Instance directory not found"
	exit 1
fi


# We recommend using a multiple of the number of instance suffixes (10 for indepset, 16 for set cover), as it otherwise
# may unbalance the runs

ninsts=`ls instances/indepset/random_400_80*.clq | wc -l`   # number of instance suffixes
nsets=$((($ninsts + ($1 - 1)) / $1))                        # number of sets of runs ($ninsts / $1 rounded up)

for j in `seq 0 $(($nsets-1))`; do
	for i in `seq 1 $1`; do
		inst_id=$(($j * $1 + $i - 1))
		if [ $inst_id -gt $ninsts ]; then
			break
		fi

		# Run script
		bash run_indepset_single.sh $inst_id&

	done
	wait
done
