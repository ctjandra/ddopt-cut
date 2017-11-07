#!/bin/bash

# Bash script to run target cut experiments for the independent set problem
# This takes as argument the instance number; use run_setcover.sh to run several at once

# Experiments run by this script:
# * Solving time + nodes [bw50, bw60] [bw40 appendix]
# * Gap closed [bw40]
# * Face dimensions [bw40]
# * Target vs Lagrangian cuts [bw50]


instance_dir="instances/setcover"
ddopt_bin="bin/ddopt"
global_params="--cut-intpt 1"

if [ -z "$1" ]; then
        echo "Error: No argument supplied; argument indicates instance suffix (e.g. 0 means run for *_0.mps instances)"
        exit 1
fi

if [ ! -d output ]; then
	mkdir output
fi

if [ ! -d output/setcover_main ]; then
        mkdir output/setcover_main
fi

if [ ! -d output/setcover_gap ]; then
        mkdir output/setcover_gap
fi

if [ ! -d output/setcover_dim ]; then
        mkdir output/setcover_dim
fi

run () # $1 = arguments (including instance), $2 = complete output filename (with path)
{
        output_filename=$2
        output_path=output/${output_filename}
        if [ ! -e ${output_path} ]; then
                ${ddopt_bin} ${global_params} $1 > ${output_path}
        fi
}

small_instances_params="ss30_bw40"
large_instances_params="ss30_bw50 ss30_bw60"
all_params="$small_instances_params $large_instances_params"


# -- setcover_main experiments
# Small and large instances: Standard runs and Lagrangian cuts
for instance_params in ${all_params}; do
        for inst in ${instance_dir}/sc_n250_${instance_params}_${1}.mps; do

                # No DD cuts; pure solve, for baseline comparisons
                run "--skip-dd ${inst}" "setcover_main/`basename ${inst} .mps`_noddcuts_nocplexcuts.log"
                run "--solver-cuts 0 --skip-dd ${inst}" "setcover_main/`basename ${inst} .mps`_noddcuts_cplexcuts.log"

                for width in 100 300 500; do
                        for ncuts in 1 3 5 10; do
                                run "-c ${ncuts} -w ${width} ${inst}" "setcover_main/`basename ${inst} .mps`_w${width}_c${ncuts}.log"
                                run "--solver-cuts 0 -c ${ncuts} -w ${width} ${inst}" "setcover_main/`basename ${inst} .mps`_w${width}_c${ncuts}_cplexcuts.log"

                                if [ ${instance_params} == "ss30_bw50" ]; then
                                        run "--obj-cut --cut-lagrangian -c ${ncuts} -w ${width} ${inst}" "setcover_main/`basename ${inst} .mps`_w${width}_c${ncuts}_lroc.log"
                                        run "--obj-cut-after -c ${ncuts} -w ${width} ${inst}" "setcover_main/`basename ${inst} .mps`_w${width}_c${ncuts}_objcut.log"
                                fi
                        done

                        if [ ${instance_params} == "ss30_bw50" ]; then
                                run "--obj-cut -w ${width} ${inst}" "setcover_main/`basename ${inst} .mps`_w${width}_c0_nocutsoc.log"
                        fi

                done
        done
done


# -- setcover_gap experiments
# Small instances: Gap closed up to 30 cuts
for instance_params in ${small_instances_params}; do
        for inst in ${instance_dir}/sc_n250_${instance_params}_${1}.mps; do
                # No DD cuts; pure solve, for baseline comparisons
                run "--skip-dd ${inst}" "setcover_gap/`basename ${inst} .mps`_noddcuts_nocplexcuts.log"
                run "--solver-cuts 0 --skip-dd ${inst}" "setcover_gap/`basename ${inst} .mps`_noddcuts_cplexcuts.log"

                base=`basename ${inst} .mps`
                width=`cat ${instance_dir}/${base}.width`

                for widthfactor in 1 5 10 20 40 60 80 100; do
                        real_width=$((${width} * ${widthfactor} / 100))

                        # Experiment: Gap closed
                        run "-w ${real_width} -c 30 ${inst}" "setcover_gap/`basename ${inst} .mps`_f${widthfactor}_manycuts.log"
                done
        done
done


# -- setcover_dim experiments
# Small instances: Face dimensions (with or without perturbation)
for instance_params in ${small_instances_params}; do
        for inst in ${instance_dir}/sc_n250_${instance_params}_${1}.mps; do

                base=`basename ${inst} .mps`
                width=`cat ${instance_dir}/${base}.width`

                for widthfactor in 1 5 10 20 40 60 80 100; do
                        real_width=$((${width} * ${widthfactor} / 100))

                        # Experiment: Face dimensions
                        run "--root-only --cut-flow-decomposition -c 10 -w ${real_width} ${inst}" "setcover_dim/`basename ${inst} .mps`_f${widthfactor}_facedims_nopert.log"

                        # Unused
                        # Experiment: Face dimensions with perturbation
                        # run "--cut-perturbation --root-only --cut-flow-decomposition -c 10 -w ${real_width} ${inst}" "setcover_dim/`basename ${inst} .mps`_f${widthfactor}_facedims_wpert.log"
                done
        done
done
