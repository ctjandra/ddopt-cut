# In summary, this script performs the following operations:
# 1. Extract relevant data from raw output files through a bash command (possibly externally through ssh)
# 2. Aggregate data and average results
# 3. Plot graphs of the results
# Running this script remotely (i.e. not on the machine containing the output files) requires the paramiko module.

# Warning: If new bash commands are added, they must be tested before use; there is no error detection for them.

################################################################################################

import collections
import numpy
import os
import sys

import data_collection
from settings import *
from data_plotter_setcover import *
from data_plotter_indepset import *


plotters = {
	'setcover_tn'  : PlotterSetCoverTimeNodes(),
	'setcover_lr'  : PlotterSetCoverLagrangian(),
	'setcover_gap' : PlotterSetCoverGap(),
	'setcover_dim' : PlotterSetCoverDimensions(),

	'indepset_tn'  : PlotterIndepSetTimeNodes(),
	'indepset_lr'  : PlotterIndepSetLagrangian(),
	'indepset_gap' : PlotterIndepSetGap(),
	'indepset_dim' : PlotterIndepSetDimensions(),
	'indepset_bd'  : PlotterIndepSetTimeBreakdown(),
}


def print_parameter_options():
	print
	print '    setcover_tn:   Plot solving time and number of nodes for set covering.     Requires setcover_main experiments.'
	print '    setcover_lr:   Plot Lagrangian cut comparison for set covering.            Requires setcover_main experiments.'
	print '    setcover_gap:  Plot gap closed for set covering.                           Requires setcover_gap experiments.'
	print '    setcover_dim:  Plot face dimensions for set covering.                      Requires setcover_dim experiments.'
	print
	print '    indepset_tn:   Plot solving time and number of nodes for independent set.  Requires indepset_main experiments.'
	print '    indepset_lr:   Plot Lagrangian cut comparison for independent set.         Requires indepset_main experiments.'
	print '    indepset_gap:  Plot gap closed for independent set.                        Requires indepset_gap experiments.'
	print '    indepset_dim:  Plot face dimensions for independent set.                   Requires indepset_dim experiments.'
	print '    indepset_bd:   Plot solving time breakdown for independent set.            Requires indepset_main experiments.'
	print
	print '    all:           Plot all of the above.                                      Requires all of the above experiments.'
	print


def run_plotter(plotter, ssh=None):
	# Prepare parameters from plotter
	fullpath = PATH + '/' + plotter.run_directory
	selected_fields = {k : FIELDS[k] for k in plotter.required_fields if k in FIELDS}
	selected_additional_fields = {k : ADDITIONAL_FIELDS[k] for k in plotter.required_fields if k in ADDITIONAL_FIELDS}
	instance_index = INSTANCE_INDICES[plotter.instance_type]
	try: # filename filter allows us to only look at specific files from the directory
		selected_filename_filter = plotter.filename_filter
	except AttributeError:
		selected_filename_filter = None
	try:
		selected_filename_filter_per_field = plotter.filename_filter_per_field
	except AttributeError:
		selected_filename_filter_per_field = None

	# Collect data
	data, avg_data = data_collection.collect_data(selected_fields, instance_index, fullpath, USERHOST, selected_additional_fields,
		selected_filename_filter, selected_filename_filter_per_field, ssh)

	if not os.path.exists(OUTPUT_DIR):
		os.makedirs(OUTPUT_DIR)

	# Plot data
	plotter.plot(data, avg_data, OUTPUT_DIR)


# Main script

if len(sys.argv) <= 1 or (sys.argv[1] not in plotters and sys.argv[1] != 'all'):
	print 'Parameter not supplied or invalid. Options:'
	print_parameter_options()
	sys.exit(1)

if sys.argv[1] == 'all':
	ssh = None
	if USERHOST != "":
		username, hostname = USERHOST.split('@')
		ssh = data_collection.ssh_connect(username, hostname)

	for name, plotter in sorted(plotters.iteritems()):
		print 'Running ' + name + '...'
		run_plotter(plotter, ssh)

	if ssh:
		ssh.close()
else:
	plotter = plotters[sys.argv[1]]
	run_plotter(plotter)
