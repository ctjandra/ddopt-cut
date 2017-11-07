#include <fstream>
#include <iostream>
#include <sstream>

#include "getopt.h"
#include "main_prob.hpp"
#include "util/options.hpp"

using namespace std;


int main(int argc, char* argv[])
{

	Options options;

	if (argc < 2) {
		cout << "\nUsage: " << argv[0] << " [options] [instance file]\n";
		cout << endl;

		cout << "Execution options:\n";
		cout << "    --dd-only                 do not run the IP solver\n";
		cout << endl;

		cout << "Decision diagram construction options:\n";
		cout << "    -m [id]                   merging scheme (see documentation for ids)\n";
		cout << "    -o [id]                   variable ordering (see documentation for ids)\n";
		cout << "    -w [width]                maximum decision diagram width (default: no limit)\n";
		cout << endl;

		cout << "Decision diagram cut options:\n";
		cout << "    -c [ncuts]                limit of number of DD cuts generated (default: " <<
		     options.limit_ncuts << ")\n";
		cout << "    --cut-perturbation        perturb target cut randomly to increase its dimension\n";
		cout << "    --cut-lagrangian          generate Lagrangian cuts instead of target cuts\n";
		cout << "    --cut-intpt [id]          select interior point for target cuts (see documentation for ids)\n";
		cout << "    --obj-cut                 add objective constraint from decision diagram bound\n";
		cout << "    --cut-flow-decomposition  run flow decomposition on the generated cuts\n";
		cout << endl;

		cout << "MIP solver options:\n";
		cout << "    --solver-cuts [set]       MIP solver cuts: -1 none (default), 0: solver default, 2: aggressive\n";
		cout << "    --root-only               stop solver at the end of the root node\n";
		cout << endl;

		cout << "See documentation for additional options\n";

		cout << endl;
		exit(1);
	}

	int order_n = -1;
	int merge_n = -1;
	bool dd_only = false;
	bool skip_dd = false;

	static struct option long_options[] = {
#define OPT_NO_CUTS            1
#define OPT_DD_ONLY            2
#define OPT_NO_LONG_ARCS       3
#define OPT_OBJ_CUT            4
#define OPT_OBJ_CUT_AFTER      5
#define OPT_OBJ_CUT_VAL        6
#define OPT_SOLVER_CUTS        7
#define OPT_ROOT_ONLY          8
#define OPT_CUT_PERTURBATION   9
#define OPT_CUT_PERTURB_ITER  10
#define OPT_CUT_OBJ_WEIGHT    11
#define OPT_CUT_MAX_DEPTH     12
#define OPT_CUT_LAGRANGIAN    13
#define OPT_CUT_LAGRANGIAN_CB 14
#define OPT_CUT_FLOW_DECOMP   15
#define OPT_CUT_INTPT         16
#define OPT_SKIP_DD           17
#define OPT_ROOT_LP           18
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
		{"root-lp",                required_argument, 0, OPT_ROOT_LP},
		{0, 0, 0, 0}
	};

	int c;
	int option_index = 0;
	while ((c = getopt_long(argc, argv, "bc:m:o:w:", long_options, &option_index)) != -1) {
		switch (c) {
		case 'c':
			options.limit_ncuts = atoi(optarg);
			break;
		case 'm':
			merge_n = atoi(optarg);
			break;
		case 'o':
			order_n = atoi(optarg);
			break;
		case 'w':
			options.width = atoi(optarg);
			break;
		case OPT_NO_CUTS:
			options.generate_cuts = false;
			break;
		case OPT_DD_ONLY:
			dd_only = true;
			break;
		case OPT_NO_LONG_ARCS:
			options.use_long_arcs = false;
			break;
		case OPT_OBJ_CUT:
			options.bdd_bound_constraint = true;
			break;
		case OPT_OBJ_CUT_AFTER:
			options.bdd_bound_constraint_after_cuts = true;
			break;
		case OPT_OBJ_CUT_VAL:
			options.bdd_bound_constraint_val = atoi(optarg);
			break;
		case OPT_SOLVER_CUTS:
			options.mip_cuts = atoi(optarg);
			break;
		case OPT_ROOT_ONLY:
			options.stop_after_root = true;
			break;
		case OPT_CUT_PERTURBATION:
			options.cut_perturbation_random = true;
			if (options.cut_perturbation_iterative) {
				cout << "Error: Invalid parameter - only one type of perturbation can be on at a time" << endl;
				exit(1);
			}
			break;
		case OPT_CUT_PERTURB_ITER:
			options.cut_perturbation_iterative = true;
			if (options.cut_perturbation_random) {
				cout << "Error: Invalid parameter - only one type of perturbation can be on at a time" << endl;
				exit(1);
			}
			break;
		case OPT_CUT_OBJ_WEIGHT:
			options.cut_obj_weight = atof(optarg);
			if (options.cut_obj_weight < 0 || options.cut_obj_weight > 1) {
				cout << "Error: Invalid parameter - objective weight for cut" << endl;
				exit(1);
			}
			break;
		case OPT_CUT_MAX_DEPTH:
			options.cut_max_depth = atoi(optarg);
			if (options.cut_max_depth < 0) {
				cout << "Error: Invalid parameter - maximum depth for cut" << endl;
				exit(1);
			}
			break;
		case OPT_CUT_LAGRANGIAN:
			options.cut_lagrangian = true;
			break;
		case OPT_CUT_LAGRANGIAN_CB:
			options.cut_lagrangian_cb = true;
			break;
		case OPT_CUT_FLOW_DECOMP:
			options.cut_flow_decomposition = true;
			break;
		case OPT_CUT_INTPT:
			options.cut_interior_point = atoi(optarg);
			if (options.cut_interior_point < 0 || options.cut_interior_point > 3) {
				cout << "Error: Invalid parameter - interior point for target cut" << endl;
				exit(1);
			}
			break;
		case OPT_SKIP_DD:
			skip_dd = true;
			break;
		case OPT_ROOT_LP:
			options.root_lp = atoi(optarg);
			if (options.root_lp < 0) {
				cout << "Error: Invalid parameter - LP root algorithm" << endl;
				exit(1);
			}
			break;
		default:
			exit(1);
		}
	}

	// Check if input file is specified and exists
	if (optind >= argc) {
		cout << "Error: Input file not specified" << endl;
		exit(1);
	}
    ifstream input(argv[optind]);
    if (!input.good()) {
    	cout << "Error: Input file cannot be opened" << endl;
    	exit(1);
    }

	if (!options.generate_cuts) {
		options.limit_ncuts = 0;
	}

	// Identify problem through instance file extension
	string instance_path = string(argv[optind]);
	string instance_filename = instance_path.substr(instance_path.find_last_of("\\/") + 1);
	string instance_extension = instance_filename.substr(instance_filename.find_last_of(".") + 1);
	if (instance_extension == "clq") {
		main_indepset(order_n, merge_n, instance_path, instance_filename, skip_dd, dd_only, options);
	} else if (instance_extension == "mps") {
		main_bp(order_n, merge_n, instance_path, instance_filename, skip_dd, dd_only, options);
	} else {
		cout << "Error: Problem type (" << instance_extension << ") not identified" << endl;
		exit(1);
	}

	return 0;
}
