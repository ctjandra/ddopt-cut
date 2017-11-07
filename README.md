Target Cuts from Relaxed Decision Diagrams
==========================================

This is the implementation used to produce the computational results in the following paper:

* C. Tjandraatmadja, W.-J. van Hoeve, [*"Target Cuts from Relaxed Decision Diagrams"*](http://www.andrew.cmu.edu/user/ctjandra/pdfs/Target_Cuts_from_DDs_Preprint.pdf). Accepted to INFORMS Journal of Computing.

In this paper and implementation, we generate target cuts from relaxed decision diagrams, using as benchmarks the independent set problem and the set covering problem. This code in fact supports general pure binary problems in MPS format. In a nutshell, we compactly encode a relaxation of the feasible set of the problem using decision diagrams and extract inequalities that define facets of the relaxation and separate a given point from the feasible set.

The main requirements are CPLEX 12.6 and a C++ compiler in a Linux environment. Python 2.7 with numpy and matplotlib is required to process data and plot graphs. Boost is used but included with the code.

**Please be aware that the code quality is at research level and this repository is not expected to be maintained. Its main purpose is to allow and facilitate the reproducibility of the results contained in the above paper.** However, you are free to adapt it to your own purposes; see `LICENSE` for more details. Updated or repurposed versions of this code may appear elsewhere in the future.

Instructions for reproducing the experiments are below. For information about the options supported by the program, see [OPTIONS.md](OPTIONS.md). For information about the plots that can be reproduced with this code, see [EXPERIMENTS.md](EXPERIMENTS.md).

Instructions
------------

In order to reproduce the results, you need to perform the following three steps after downloading the package and unpacking it:

1. Compile the code;
2. Run the test scripts; and
3. Run the plotting scripts.

The test scripts are Bash scripts that run all experiments involved in the paper (optionally in parallel). The plotting scripts are Python scripts that processes the output and plots the graphs shown in the paper.

The other files we include are the instances we use (with scripts to generate them) and abridged output files from our actual runs. We do not include full output files due to their size (> 2GB); the abridged output files contain only the lines that are parsed by the scripts.

You should unpack the package in the machine you will run the experiments; however, the plotting scripts support remote access via ssh and can be used in a different machine. See further below for more details.


### Compiling the code

The code is in the directory `ddopt`. To compile it, follow these steps:

1. Ensure you have all dependencies: CPLEX 12.6 (other versions may work) and a C++ compiler. Boost is used but it is packaged with the code.

2. Configure the CPLEX path by opening the `Makefile` file and setting the variable `BASEILOGDIR` to the path where CPLEX is installed in your machine.

3. Run `make` to compile the code.

    *Note: The Makefile will unpack the boost dependencies from `boost_deps.tar.gz` if needed. The boost license file is within the package.*


### Running the test scripts

The test scripts are in the directory `test`. You must have compiled the code to perform this step. The Makefile will automatically copy the binary to the `tests/bin` directory, which will be used by the test scripts.

*Note: If you want to run a smaller test, run `./reduce_instance_set.sh` in order to keep only the first 3 numbered instances.*

1. To run the independent set experiments, run `./run_indepset.sh [n]`, where `n` is the number of processes that will be run. The parallelization is simple: we assign one process per instance number. Since for independent set there are 10 instance numbers, we recommend setting `n` to a divisor of 10.

2. To run the set covering experiments, run `./run_setcover.sh [n]`, where `n` is the number of processes that will be run. Similarly to above, we recommend setting `n` to a divisor of 16 as there are 16 instance numbers for set covering.


### Running the plotting scripts

The plotting scripts are in the directory `test`. You must have run the experiments to completion to perform this step, which may take days. However, these scripts also work with the abridged output files included in the package. The scripts require numpy, matplotlib, and optionally paramiko if remote access is used.

1. Configure the plotting settings. These are at the top of `settings.py` and there are four options that can be configured:

    * `USERHOST`: If the files are in a remote server, this is the address of the server in the format `"[username]@[hostname]"`. The server is accessed via ssh. If the files are available locally, leave this as is (as an empty string `""`).

    * `PATH`: This is the path of the output files, whether locally or in a server.

    * `OUTPUT_DIR`: This is the directory where the plots will be created.

    * `PLOT_COLOR`: Set this to True if plots should be colored.

    Additionally, if you are plotting gap closed for small set cover instances, copy the .opt files from the `instances/setcover` directory to the `output/setcover_gap` output directory.

2. Run `python plot_results.py all`.

This will process the output files and generate all plots. Specific plots can be generated by providing different arguments; run `python plot_results.py` without arguments to obtain a list.


### Other files

Other than the code and the scripts included above, we include:

* **Instances:** The directories `tests/instances/indepset` and `tests/instances/setcover` respectively contain the independent set and set covering instances used for the experiments.

    * **Instance generation:** The directory `tests/instances/scripts` includes the Python scripts used to generate instances,  `generate_indepset_instances.py` and `generate_setcover_instances.py`. The set covering script requires CPLEX and by default also generates .opt files with the optimal value for bandwidth 40 instances. The script `extract_exact_width.sh` runs the program to write exact decision diagram widths to .width files for all files given as arguments. These .opt (for set covering) and .width files are necessary for tests with the smaller instances.

* **Abridged output files:** The package `abridged_output.tar.gz` in the root directory contains shortened versions of the output files that were used for plotting the graphs in the paper. The full output is not included due to their size (> 2GB). These output files contain only the lines parsed by the plotting scripts. It also includes the script `abridge_log_files.sh` used to obtain these files. In order to generate the graphs from these files, unpack it and configure `settings.py` to read from the `abridged_output` directory.

* **Affine independence verifier:** The affine independence property of the points provided by flow decomposition is proven in the paper. We use the external Python script `verify_dimension.py` in the `tests` directory to assert that this holds true.


Acknowledgments
---------------

This implementation is based off the code from the following paper:

* D. Bergman, A. A. Cire, W.-J. van Hoeve, and J. N. Hooker. [*Optimization Bounds from Binary Decision Diagrams*](https://doi.org/10.1287/ijoc.2013.0561). INFORMS Journal on Computing, 26(2):253--268.

It was heavily modified since then. Their original code is available [here](http://www.andrew.cmu.edu/user/vanhoeve/mdd/code/opt_bounds_bdd-src.tar.gz).
