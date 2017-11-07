Program options
===============

This is a list of the command line arguments that can be passed to the program.

```
Usage: ./ddopt [options] [instance file]

Execution options:
    --dd-only                 do not run the IP solver
    --skip-dd                 run only the IP solver and do not construct decision diagrams

Decision diagram construction options:
    -m [id]                   merging scheme (see below for ids)
    -o [id]                   variable ordering (see below for ids)
    -w [width]                maximum decision diagram width (default: no limit)
    --no-long-arcs            do not use long arcs in the construction

Decision diagram cut options:
    -c [ncuts]                limit of number of DD cuts generated (default: 0)
    --cut-perturbation        perturb target cut randomly to increase its dimension
    --cut-lagrangian          generate Lagrangian cuts instead of target cuts
    --cut-lagrangian-cb       generate Lagrangian cuts instead of target cuts using the ConicBundle library
    --cut-intpt [id]          select interior point for target cuts (see below)
    --obj-cut                 add objective constraint from decision diagram bound
    --obj-cut-val [val]       add objective constraint with specific value
    --cut-max-depth [d]       maximum depth in which cuts are generated
    --cut-flow-decomposition  run flow decomposition on the generated cuts

MIP solver options:
    --solver-cuts [set]       MIP solver cuts: -1 none (default), 0: solver default, 2: aggressive
    --root-only               stop solver at the end of the root node
```

Note: While you need CPLEX to reproduce the results of the paper, the construction of a decision diagram for independent set instances can still be done without CPLEX. The code is designed to compile and run without CPLEX by setting the option `USE_CPLEX` in the Makefile to 0. You must however run the program with the `--dd-only` argument.


Merging and ordering options
----------------------------

Several of these merging and ordering rules are inherited from the original code acknowledged in [README.md](README.md).

### Independent set

The merging rule options for `-m` for independent set are the following:

1. Minimum longest path
2. Pairwise minimum longest path
3. Consecutive pairs minimum longest path
4. Minimum state size
5. Maximum state size
6. Lexicographic
7. Minimum dual bound of merged node
8. Random

The default and the one used in the paper is 1, minimum longest path.

The variable ordering options for `-o` for independent set are the following:

1. Random
2. Maximal path decomposition
3. Vertex in the least number of states
4. Cut vertex decomposition with respect to a spanning tree
5. Cut vertex decomposition
6. Vertex in the least number of states with randomization
7. Fixed order (reads the order from `fixed_order.txt`)
8. Minimum degree
9. No ordering

The default and the one used in the paper is 8, minimum degree. 


### Set covering / binary programs

The merging rule options for `-m` for set covering (and binary programs in general) are the following:

1. Minimum longest path
2. Pairwise minimum longest path
3. Consecutive pairs minimum longest path
4. Lexicographic
5. Random

The default and the one used in the paper is 1, minimum longest path.

The variable ordering options for `-o` for set covering (and binary programs in general) are the following:

1. Random
2. Cuthill-McKee
3. Fixed order (reads the order from `fixed_order.txt`)
4. No ordering

The default and the one used in the paper is 4, no ordering.


Interior point options
----------------------

This selects the interior point used for generating target cuts. It is responsibility of the user to select a point that is valid for the problem.

1. Zero vector (0,0,...,0)
2. One vector (1,1,...,1)
3. Close to zero vector (1/(2n), 1/(2n), ..., 1/(2n))
4. Center of the decision diagram

Options 1, 2, and 3 are problem-dependent; i.e. they can only be used if we are sure the point falls into the feasible set. It is also acceptable if the point falls into the boundary of the feasible set, though as a consequence any hyperplane supported by the point will not be generated.

Option 4 is a general option that can be used for any problem. It requires the library GMP to be compiled with the code due to the use of large numbers; the setting `USE_GMP` in the Makefile must be set to 1 and the library must be available in the system.

By default, the independent set case uses option 3 and the binary problem case uses option 4. The scripts for set covering use option 2.


<!-- 
		{"merger",                 required_argument, 0, 'm'},
		{"ordering",               required_argument, 0, 'o'},
		{"width",                  required_argument, 0, 'w'},
		{"ncuts",                  required_argument, 0, 'c'},
		{"no-cuts",                no_argument,       0, OPT_NO_CUTS},
		{"dd-only",                no_argument,       0, OPT_DD_ONLY},
		{"no-long-arcs",           no_argument,       0, OPT_NO_LONG_ARCS},
		{"obj-cut",                no_argument,       0, OPT_OBJ_CUT},
		{"obj-cut-after",          no_argument,       0, OPT_OBJ_CUT_AFTER},
		{"obj-cut-val",            required_argument, 0, OPT_OBJ_CUT_VAL},
		{"solver-cuts",            required_argument, 0, OPT_SOLVER_CUTS},
		{"root-only",              no_argument,       0, OPT_ROOT_ONLY},
		{"cut-perturbation",       no_argument,       0, OPT_CUT_PERTURBATION},
		{"cut-perturbation-iter",  no_argument,       0, OPT_CUT_PERTURB_ITER},
		{"cut-obj-weight",         required_argument, 0, OPT_CUT_OBJ_WEIGHT},
		{"cut-max-depth",          required_argument, 0, OPT_CUT_MAX_DEPTH},
		{"cut-lagrangian",         no_argument,       0, OPT_CUT_LAGRANGIAN},
		{"cut-lagrangian-cb",      no_argument,       0, OPT_CUT_LAGRANGIAN_CB},
		{"cut-flow-decomposition", no_argument,       0, OPT_CUT_FLOW_DECOMP},
		{"cut-intpt",              required_argument, 0, OPT_CUT_INTPT},
		{"skip-dd",                no_argument,       0, OPT_SKIP_DD},
		{"root-lp",                required_argument, 0, OPT_ROOT_LP}, -->