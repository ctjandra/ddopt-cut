
######## User configuration ####################################################################

# For purposes of reproducing experiments, this section is the only one that needs configuration

# USERHOST must be of the form username@hostname; if empty, will access path locally
USERHOST = ""
PATH = "output"

# Output directory
OUTPUT_DIR = 'plots'

# If true, plots are created in color
PLOT_COLOR = False



######## In-depth configuration ################################################################

# Extends a list of size below n up to n by repeating the last element; used for defining some fields below
def extend_list_by_repeating_last(lst, n):
	assert len(lst) > 0 and len(lst) <= n
	return lst + [lst[-1] for i in range(n - len(lst))]

# Bash commands that extract a value from a file (must result in a single line, rest is ignored).
# The command is piped in from cat and run directly from the shell. This implies that any command
# is accepted including pipes, but beware that this convenience turns this method *unsafe*.
# (Also, do not use single quotes and escape $'s.)
# The second value is a function applied to the resulting string (e.g. int, float, user function).
# An optional third value is True if we use cat ${f} before the command, or False if we only run the command itself
# (where ${f} would be the filename; default is True)
FIELDS = {
	'width': ('grep "[-] width" | awk "{print \$6}"', int),
	'nodes': ('grep "Number of nodes" | awk "{print \$4}"', int),
	'nusercuts': ('grep "User cuts applied:" | awk "{print \$4}"', int),
	'obj': ('grep "CPLEX obj:" | awk "{print \$3}"', float),
	'bddtime': ('grep "Time to .* BDD:" | awk "{print \$5}"', float), # Note: There is an inconsistency in the string
	'cplextime': ('grep "Total (root" | awk "{print \$4}"', float),
	'roottime': ('grep -A 1 "Root node processing" | tail -n 1 | awk "{print \$4}"', float),
	'bbtime': ('grep -A 1 "Sequential" | tail -n 1 | awk "{print \$4}"', float),
	'firstcuttime': ('grep "Time to generate cut" | awk "{print \$5}" | head -n 1; echo', float),
	'totalcuttime': ('grep "Time to generate cut" | awk "{print \$5}" | paste -s -d " " | tr " " "+" | bc -l; echo', float),

	# Gap closed
	'allgapclosed': ('rootbd=`grep "      0     0" ${f} | head -n 1 | awk "{print \\\\$3}"`;'
		+ 'cutbds=`grep "      0     2\|User:" ${f} | awk "{print \\\\$3}" | tr "\n" " "; echo`;'
		+ 'obj=`grep "CPLEX obj" ${f} | awk "{print \\\\$3}"`;'
		+ 'gaps=`for cutbd in ${cutbds}; do '
		+ '    echo "(${rootbd} - ${cutbd}) / (${rootbd} - ${obj})" | bc -l;'
		+ 'done;`;'
		+ 'echo ${gaps}',
		lambda s: extend_list_by_repeating_last(map(float, list(s.split())), 30), False), # hardcoded for 30 cuts

	# Gap closed with external optimal value file due to time limit
	'allgapclosed_optfile': ('rootbd=`grep "      0     0" ${f} | head -n 1 | awk "{print \\\\$3}"`;'
		+ 'cutbds=`grep "User:" ${f} | grep -v "integral" | awk "{print \\\\$3}" | tr "\n" " "; echo`;'
		+ 'optname=`dirname ${f}`/`basename ${f} | cut -d. -f1 | cut -d_ -f1-5`.opt; obj=`cat ${optname}`;'
		+ 'gaps=`for cutbd in ${cutbds}; do '
		+ '    echo "(${rootbd} - ${cutbd}) / (${rootbd} - ${obj})" | bc -l;'
		+ 'done;`;'
		+ 'echo ${gaps}',
		lambda s: extend_list_by_repeating_last(map(float, list(s.split())), 30), False), # hardcoded for 30 cuts
	'allgapclosed_cplex_optfile': ('rootbd=`grep "      0     0" ${f} | head -n 1 | awk "{print \\\\$3}"`;'
		+ 'cutbds=`grep "      0     2" ${f} | grep -v "integral" | awk "{print \\\\$3}" | tr "\n" " "; echo`;'
		+ 'optname=`dirname ${f}`/`basename ${f} | cut -d. -f1 | cut -d_ -f1-5`.opt; obj=`cat ${optname}`;'
		+ 'gaps=`for cutbd in ${cutbds}; do '
		+ '    echo "(${rootbd} - ${cutbd}) / (${rootbd} - ${obj})" | bc -l;'
		+ 'done;`;'
		+ 'echo ${gaps}',
		lambda s: extend_list_by_repeating_last(map(float, list(s.split())), 30), False), # hardcoded for 30 cuts

	# Flow decomposition fields
	# Hardcoded for 10 cuts
	'flowdec_feas': ('grep "Feasible solutions" ${f} | awk "{print \$5}" | tr "\n" " "; echo',
		lambda s: extend_list_by_repeating_last(map(int, list(s.split())), 10), False),
	'flowdec': ('grep "Feasible solutions" ${f} | awk "{print \$7}" | tr "\n" " "; echo',
		lambda s: extend_list_by_repeating_last(map(int, list(s.split())), 10), False),
}

# Fields composed of collected data
ADDITIONAL_FIELDS = {
	'totaltime': lambda d, instance: d['cplextime'][instance] + d['bddtime'][instance] if 'bddtime' in d and instance in d['bddtime'] else d['cplextime'][instance]
}

# We assume output filenames are divided by _'s. The following is the index of the part of the file name
# indicating instance number so the data can be averaged out among all instances.
# E.g. If instances are named "random_300_80_0", "random_300_80_1", "random_300_80_2", etc. then
# its INSTANCE_INDEX is 3 and results from "random_300_80_*" will be averaged.
INSTANCE_INDICES = {'indepset' : 3, 'setcover': 4}


######## Plot-related settings #################################################################

import matplotlib
import matplotlib.pyplot as plt
import pylab
import re

def plot_dict(d, label, marker='o', color=None, linestyle='-', linewidth=1.5):
	if not any(d):
		return # do nothing if dict is empty
	it = sorted([(k, v) for k, v in d.items() if v is not None])
	plt.plot([i[0] for i in it], [i[1] for i in it], linestyle=linestyle, label=label, marker=marker, color=color,
		linewidth=linewidth, solid_capstyle="round")

def natural_sort_key(key):
	return [int(c) if c.isdigit() else c for c in re.split('([0-9]+)', key)]

if PLOT_COLOR:
	colors = ["#E41A1C", "#377EB8", "#4DAF4A", "#FF7F00", "#984EA3", "#777777", "#A65628", "#F781BF", "#BFBF26"]
	markers = ['^'] * 10
else:
	colors = ["#000000"] * 10
	markers = ['*', '^', 'o', 's', 'v', 'D', 'x', '+', '_', '.']

matplotlib.rcParams.update({'font.size': 14})

labels = {
	'ncuts'     : 'Number of cuts',
	'nodes'     : 'Number of nodes',
	'time'      : 'Time (s)',
	'totaltime' : 'Total solving time (s)',
	'bddtime'   : 'Time (s)',
	'cplextime' : 'Time (s)',
	'roottime'  : 'Time (s)',
	'bbtime'    : 'Time (s)',
	'width'     : 'Width',
	'gap'       : 'Gap closed (%)'
}


######## Filters and selectors #################################################################

# Reconstructs instance name
setcover_instance_name = lambda k : '_'.join(k[0:4])
indepset_instance_name = lambda k : '_'.join(k[0:3])

# Note: Keep in mind that the instance number is removed from the key

# Functions to select output filenames: Set covering
key_filters_setcover = {
	'standard' : (lambda k : len(k) == 6 and k[4].startswith('w') and k[5].startswith('c')), # Standard run
	'standard_nocuts' : (lambda k: len(k) == 6 and k[4] == 'noddcuts' and k[5] == 'nocplexcuts'), # Standard run with zero BDD cuts
	'standard_nocuts_sc0' : (lambda k: len(k) == 6 and k[4] == 'noddcuts' and k[5] == 'cplexcuts'), # Standard run with zero BDD cuts + CPLEX cuts

	'lroc' : (lambda k: len(k) == 7 and k[4].startswith('w') and k[5].startswith('c') and k[6] == 'lroc'), # Lagrangian with objective bound
	'lrcb' : (lambda k: len(k) == 7 and k[4].startswith('w') and k[5].startswith('c') and k[6] == 'lrcb'), # Lagrangian with ConicBundle
	'objcut' : (lambda k: len(k) == 7 and k[4].startswith('w') and k[5].startswith('c') and k[6] == 'objcut'), # Objective bound
	'nocutsoc' : (lambda k: len(k) == 7 and k[4].startswith('w') and k[5] == 'c0' and k[6] == 'nocutsoc'), # Objective bound only (no cuts)

	'gap_runs' : (lambda k: len(k) == 6 and k[5] == 'manycuts'), # Run to compute gap closed

	'flowdec_pert' : (lambda k: len(k) == 7 and k[6] == 'wpert'), # Flow decomposition with perturbation
	'flowdec' : (lambda k: len(k) == 7 and k[6] == 'nopert'), # Flow decomposition without perturbation
}

# Functions to select output filenames: Independent set
key_filters_indepset = {
	'standard' : (lambda k : len(k) == 5 and k[3].startswith('w') and k[4].startswith('c')), # Standard run
	'standard_nocuts' : (lambda k: len(k) == 5 and k[3] == 'noddcuts' and k[4] == 'nocplexcuts'), # Standard run with zero BDD cuts
	'standard_nocuts_sc0' : (lambda k: len(k) == 5 and k[3] == 'noddcuts' and k[4] == 'cplexcuts'), # Standard run with zero BDD cuts + CPLEX cuts

	'lroc' : (lambda k: len(k) == 6 and k[3].startswith('w') and k[4].startswith('c') and k[5] == 'lroc'), # Lagrangian (independent set)
	'objcut' : (lambda k: len(k) == 6 and k[3].startswith('w') and k[4].startswith('c') and k[5] == 'objcut'), # Objective bound
	'nocutsoc' : (lambda k: len(k) == 6 and k[3].startswith('w') and k[4] == 'c0' and k[5] == 'nocutsoc'), # Objective bound only (no cuts)

	'gap_runs' : (lambda k: len(k) == 5 and k[4] == 'manycuts'), # Run to compute gap closed

	'flowdec_pert' : (lambda k: len(k) == 6 and k[5] == 'wpert'), # Flow decomposition with perturbation
	'flowdec' : (lambda k: len(k) == 6 and k[5] == 'nopert'), # Flow decomposition without perturbation
}

# Functions to select parameters from output filename: Set covering
key_selectors_setcover = {
	'width' : (lambda k : int(k[4][1:])),
	'ncuts' : (lambda k : int(k[5][1:])),
	'width_factor' : (lambda k : int(k[4][1:])),
}

# Functions to select parameters from output filename: Independent set
key_selectors_indepset = {
	'width' : (lambda k : int(k[3][1:])),
	'ncuts' : (lambda k : int(k[4][1:])),
	'width_factor' : (lambda k : int(k[3][1:])),
}
