#!/bin/bash

# Bash script to run target cut experiments for the independent set problem
# This takes as argument the instance number; use run_indepset.sh to run several at once

# Experiments run by this script:
# * Solving time + nodes + time breakdown [large instances]
# * Gap closed [small instances]
# * Face dimensions [small instances]
# * Target vs Lagrangian cuts [large instances]
# * Perturbation heuristic [small instances]


instance_dir="instances/indepset"
ddopt_bin="bin/ddopt"
global_params="--root-lp 4"

if [ -z "$1" ]; then
        echo "Error: No argument supplied; argument indicates instance suffix (e.g. 0 means run for *_0.clq instances)"
        exit 1
fi

if [ ! -d output ]; then
        mkdir output
fi

if [ ! -d output/indepset_main ]; then
        mkdir output/indepset_main
fi

if [ ! -d output/indepset_gap ]; then
        mkdir output/indepset_gap
fi

if [ ! -d output/indepset_dim ]; then
        mkdir output/indepset_dim
fi

run () # $1 = arguments (including instance), $2 = complete output filename (with path)
{
        output_filename=$2
        output_path=output/${output_filename}
        if [ ! -e ${output_path} ]; then
                ${ddopt_bin} ${global_params} $1 > ${output_path}
        fi
}

small_instances_params="300_80 120_50"
large_instances_params="400_80 250_50"
all_params="$small_instances_params $large_instances_params"


# -- indepset_main experiments
# Large instances: Standard runs and Lagrangian cuts
for instance_params in ${large_instances_params}; do
        for inst in ${instance_dir}/random_${instance_params}_${1}.clq; do

                if [[ ${instance_params} == "400_80" ]]; then
                        widths="100 200 400 600 100000" # 100000 is treated as exact
                fi
                if [[ ${instance_params} == "250_50" ]]; then
                        widths="500 1000 1500 2000 2500"
                fi

                # No DD cuts; pure solve, for baseline comparisons
                run "--skip-dd ${inst}" "indepset_main/`basename ${inst} .clq`_noddcuts_nocplexcuts.log"
                run "--solver-cuts 0 --skip-dd ${inst}" "indepset_main/`basename ${inst} .clq`_noddcuts_cplexcuts.log"

                # Experiment: Solving time and number of nodes
                for width in ${widths}; do
                        for ncuts in 1 3 5 10; do
                                run "-c ${ncuts} -w ${width} ${inst}" "indepset_main/`basename ${inst} .clq`_w${width}_c${ncuts}.log"
                                run "--solver-cuts 0 -c ${ncuts} -w ${width} ${inst}" "indepset_main/`basename ${inst} .clq`_w${width}_c${ncuts}_cplexcuts.log"

                                # Lagrangian relaxation + comparison with objective cut
                                run "--obj-cut --cut-lagrangian -c ${ncuts} -w ${width} ${inst}" "indepset_main/`basename ${inst} .clq`_w${width}_c${ncuts}_lroc.log"
                                run "--obj-cut -c ${ncuts} -w ${width} ${inst}" "indepset_main/`basename ${inst} .clq`_w${width}_c${ncuts}_objcut.log"
                        done

                        run "--obj-cut -c 0 -w ${width} ${inst}" "indepset_main/`basename ${inst} .clq`_w${width}_c0_nocutsoc.log"
                done
        done
done


# -- indepset_gap experiments
# Small instances: Gap closed up to 30 cuts
for instance_params in ${small_instances_params}; do
        for inst in ${instance_dir}/random_${instance_params}_${1}.clq; do
                # No DD cuts; pure solve, for baseline comparisons
                run "--skip-dd ${inst}" "indepset_gap/`basename ${inst} .clq`_noddcuts_nocplexcuts.log"
                run "--solver-cuts 0 --skip-dd ${inst}" "indepset_gap/`basename ${inst} .clq`_noddcuts_cplexcuts.log"

                base=`basename ${inst} .clq`
                width=`cat ${instance_dir}/${base}.width`

                for widthfactor in 1 5 10 20 40 60 80 100; do
                        real_width=$((${width} * ${widthfactor} / 100))

                        # Experiment: Gap closed
                        run "-w ${real_width} -c 30 ${inst}" "indepset_gap/`basename ${inst} .clq`_f${widthfactor}_manycuts.log"
                done
        done
done


# -- indepset_dim experiments
# Small instances: Face dimensions (with or without perturbation)
for instance_params in ${small_instances_params}; do
        for inst in ${instance_dir}/random_${instance_params}_${1}.clq; do

                base=`basename ${inst} .clq`
                width=`cat ${instance_dir}/${base}.width`

                for widthfactor in 1 5 10 20 40 60 80 100; do
                        real_width=$((${width} * ${widthfactor} / 100))

                        # Experiment: Face dimensions
                        run "--root-only --cut-flow-decomposition -c 10 -w ${real_width} ${inst}" "indepset_dim/`basename ${inst} .clq`_f${widthfactor}_facedims_nopert.log"

                        # Experiment: Face dimensions with perturbation
                        run "--cut-perturbation --root-only --cut-flow-decomposition -c 10 -w ${real_width} ${inst}" "indepset_dim/`basename ${inst} .clq`_f${widthfactor}_facedims_wpert.log"
                done
        done
done
