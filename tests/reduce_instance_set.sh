#!/bin/bash

# Simple script to take only the first three instances from the instance set

if [ -d "instances_full" ]; then
        echo "Error: instances_full directory already exists; this script might have already been run"
        exit 1
fi

mv instances instances_full
mkdir instances
mkdir instances/indepset
mkdir instances/setcover
cp instances_full/indepset/*_{0,1,2}.* instances/indepset
cp instances_full/setcover/*_{0,1,2}.* instances/setcover
echo "Done: Instances moved to instances_full and the first three instances are left in the instances directory"
