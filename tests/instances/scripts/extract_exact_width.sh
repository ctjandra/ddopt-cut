#!/bin/bash

# Write exact width to a .width file for all files in arguments

if [ ! -e "../../bin/ddopt" ]; then
	echo "Error: Binary file ddopt does not exist in bin directory; make sure you have compiled the program"
	exit 1
fi

for file in $@; do
	if [ ! -e $file ]; then
		echo "Error: ${file} does not exist"
		continue
	fi
	width=`../../bin/ddopt --dd-only ${file} | grep "[-] width:" | awk '{print $6}'` 
	echo "${file} -- Exact width: ${width}"
	echo ${width} &> `basename ${file} | cut -d. -f1`.width
done
