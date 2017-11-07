import collections
import matplotlib
import matplotlib.pyplot as plt
import pylab

from settings import *


## Set covering: Solving time and number of nodes
# x-axis: Number of cuts
# y-axis: Solving time or Number of nodes
# Plot lines: Width

class PlotterSetCoverTimeNodes(object):

	instance_type = 'setcover'
	run_directory = 'setcover_main'
	required_fields = ['nodes', 'bddtime', 'cplextime', 'totaltime']
	filename_filter = '^((?!_lr|_objcut|_nocutsoc).)*$' # files that do not contain '_lr', '_objcut', '_nocutsoc'
	filename_filter_per_field = {'bddtime' : '^((?!noddcuts).)*$'} # files that do not contain 'noddcuts'

	def plot(self, data, avg_data, output_dir):
		for attr in ['nodes', 'totaltime']:

			# plot_data_all[instance_class][line][x]
			plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))

			# Extract data for ncuts >= 1
			for k, v in avg_data.iteritems():
				if key_filters_setcover['standard'](k):
					instance_class = setcover_instance_name(k)
					line = 'Width ' + str(key_selectors_setcover['width'](k))
					x_plot = key_selectors_setcover['ncuts'](k)
					plot_data_all[instance_class][line][x_plot] = v[attr][0]

			# Use nocuts data for x = 0 for all lines
			for k, v in avg_data.iteritems():
				if key_filters_setcover['standard_nocuts'](k):
					instance_class = setcover_instance_name(k)
					for line, pd in plot_data_all[instance_class].iteritems():
						pd[0] = v[attr][0]

			# Extract CPLEX without cuts and CPLEX with cuts separately
			nocutscplex_values = {}
			cutscplex_values = {}
			for k, v in avg_data.iteritems():
				if key_filters_setcover['standard_nocuts'](k):
					instance_class = setcover_instance_name(k)
					nocutscplex_values[instance_class] = v[attr][0]
				if key_filters_setcover['standard_nocuts_sc0'](k):
					instance_class = setcover_instance_name(k)
					cutscplex_values[instance_class] = v[attr][0]

			# Extract the union of the values of the xaxis, plus zero
			xaxis = [0] + sorted(list(set([key_selectors_setcover['ncuts'](k) for k in avg_data.keys() if key_filters_setcover['standard'](k)])))

			# Plot data
			for instance_class, plot_data in plot_data_all.iteritems():
				plt.clf()
				for i, (k, v) in enumerate(sorted(plot_data.iteritems())):
					plot_dict(v, k, marker=markers[i], color=colors[i])
				plt.plot(xaxis, [nocutscplex_values[instance_class] for t in range(len(xaxis))],
					label="CPLEX without cuts", linestyle='--', color="#000000", linewidth=1.5, solid_capstyle="round")
				plt.plot(xaxis, [cutscplex_values[instance_class] for t in range(len(xaxis))],
					label="CPLEX with cuts", linestyle=':', color="#000000", linewidth=1.5, solid_capstyle="round")
				plt.legend(loc=0, prop={'size':14})
				plt.ylim(bottom=0)

				# Specific cases:
				if instance_class == 'sc_n250_ss30_bw40' and attr == 'totaltime':
					plt.ylim(bottom=0, top=500)
				if instance_class == 'sc_n250_ss30_bw50' and attr == 'totaltime':
					plt.ylim(bottom=0, top=800)
				if instance_class == 'sc_n250_ss30_bw60' and attr == 'totaltime':
					plt.ylim(bottom=0, top=1000)

				plt.xlabel(labels['ncuts'])
				plt.ylabel(labels[attr])
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '.pdf')
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '.svg')



## Set covering: Lagrangian cuts
# x-axis: Number of cuts
# y-axis: Attribute
# Plot lines: Width, Target vs Lagrangian

class PlotterSetCoverLagrangian(object):

	instance_type = 'setcover'
	run_directory = 'setcover_main'
	required_fields = ['nodes', 'bddtime', 'cplextime', 'totaltime']
	filename_filter = 'bw50'

	def plot(self, data, avg_data, output_dir):
		for attr in ['nodes', 'totaltime']:

			# plot_data_all[instance_class][line][x]
			plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))

			# Extract data
			for k, v in avg_data.iteritems():
				if key_filters_setcover['objcut'](k):
					instance_class = setcover_instance_name(k)
					line = 'Target, Width ' + str(key_selectors_setcover['width'](k))
					x_plot = key_selectors_setcover['ncuts'](k)
					plot_data_all[instance_class][line][x_plot] = v[attr][0]
				if key_filters_setcover['lroc'](k):
					instance_class = setcover_instance_name(k)
					line = 'Lagrangian, Width ' + str(key_selectors_setcover['width'](k))
					x_plot = key_selectors_setcover['ncuts'](k)
					plot_data_all[instance_class][line][x_plot] = v[attr][0]

			# Use nocuts data for x = 0 for all lines (only objective bound)
			for k, v in avg_data.iteritems():
				if key_filters_setcover['nocutsoc'](k):
					instance_class = setcover_instance_name(k)
					line = 'Target, Width ' + str(key_selectors_setcover['width'](k))
					plot_data_all[instance_class][line][0] = v[attr][0]
					line = 'Lagrangian, Width ' + str(key_selectors_setcover['width'](k))
					plot_data_all[instance_class][line][0] = v[attr][0]

			# Extract CPLEX without cuts and CPLEX with cuts separately
			nocutscplex_values = {}
			cutscplex_values = {}
			for k, v in avg_data.iteritems():
				if key_filters_setcover['standard_nocuts'](k):
					instance_class = setcover_instance_name(k)
					nocutscplex_values[instance_class] = v[attr][0]
				if key_filters_setcover['standard_nocuts_sc0'](k):
					instance_class = setcover_instance_name(k)
					cutscplex_values[instance_class] = v[attr][0]

			# Extract the union of the values of the xaxis, plus zero
			xaxis = [0] + sorted(list(set([key_selectors_setcover['ncuts'](k) for k in avg_data.keys() if key_filters_setcover['objcut'](k)])))

			# Plot data
			for instance_class, plot_data in plot_data_all.iteritems():
				plt.clf()

				i_lr = 0
				i_tg = 0
				for k, v in sorted(plot_data.iteritems()):
					if "Lagrangian" in k:
						ls = '--'
						i_lr += 1
						i = i_lr - 1
					else:
						ls = '-'
						i_tg += 1
						i = i_tg - 1
					plot_dict(v, k, marker=markers[i], color=colors[i], linestyle=ls)

				plt.plot(xaxis, [nocutscplex_values[instance_class] for t in range(len(xaxis))],
					label="CPLEX without cuts", linestyle='--', color="#000000", linewidth=1.5, solid_capstyle="round")
				plt.plot(xaxis, [cutscplex_values[instance_class] for t in range(len(xaxis))],
					label="CPLEX with cuts", linestyle=':', color="#000000", linewidth=1.5, solid_capstyle="round")
				plt.legend(loc=0, prop={'size':14})
				plt.ylim(bottom=0)
				plt.xlabel(labels['ncuts'])
				plt.ylabel(labels[attr])

				if attr == 'totaltime': # Custom
					plt.ylim(bottom=0, top=1800)

				lgd = plt.legend(loc='upper center', prop={'size':12}, bbox_to_anchor=(0.5, -0.12), ncol=3)
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '_lr.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '_lr.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')



## Set covering: Gap closed
# x-axis: Number of cuts
# y-axis: Gap closed
# Plot lines: Width factor

class PlotterSetCoverGap(object):

	instance_type = 'setcover'
	run_directory = 'setcover_gap'
	required_fields = ['allgapclosed_optfile', 'allgapclosed_cplex_optfile']
	filename_filter_per_field = {'allgapclosed_cplex_optfile' : 'noddcuts_cplexcuts'}

	def plot(self, data, avg_data, output_dir):
		# plot_data_all[instance_class][line][x]
		plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))
		nocuts_data = {}

		for k, v in avg_data.iteritems():
			if key_filters_setcover['gap_runs'](k):
				instance_class = setcover_instance_name(k)
				line = 'Width ' + str(key_selectors_setcover['width_factor'](k)) + '%'
				ncuts = len(v['allgapclosed_optfile'][0])

				plot_data_all[instance_class][line][0] = 0
				for i in range(ncuts):
					plot_data_all[instance_class][line][i+1] = v['allgapclosed_optfile'][0][i] * 100

			if key_filters_setcover['standard_nocuts_sc0'](k):
				instance_class = setcover_instance_name(k)
				ncuts = len(v['allgapclosed_cplex_optfile'][0])

				plot_data_all[instance_class]['CPLEX cuts'][0] = v['allgapclosed_cplex_optfile'][0][0] * 100
				for i in range(ncuts):
					plot_data_all[instance_class]['CPLEX cuts'][i+1] = v['allgapclosed_cplex_optfile'][0][i] * 100

		for instance_class, plot_data in plot_data_all.iteritems():
			plt.clf()
			for i, k in enumerate(['Width 5%', 'Width 10%', 'Width 20%', 'Width 40%', 'Width 60%', 'Width 80%', 'Width 100%']):
				plot_dict(plot_data[k], k, marker=markers[i], color=colors[i])

			plot_dict(plot_data['CPLEX cuts'], 'CPLEX cuts', marker=None, linestyle='--', color=colors[i], linewidth=1)

			plt.legend(loc=0, prop={'size':14})
			plt.ylim(bottom=0, top=100)
			plt.xlabel(labels['ncuts'])
			plt.ylabel(labels['gap'])

			lgd = plt.legend(loc='upper center', prop={'size':12}, bbox_to_anchor=(0.5, -0.12), ncol=3)
			pylab.savefig(output_dir + '/' + instance_class + '_gapclosed.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')
			pylab.savefig(output_dir + '/' + instance_class + '_gapclosed.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')



## Set covering: Face dimensions
# x-axis: Cut number
# y-axis: Face dimension
# Plot lines: Width factor

class PlotterSetCoverDimensions(object):

	instance_type = 'setcover'
	run_directory = 'setcover_dim'
	required_fields = ['flowdec', 'flowdec_feas']
	filename_filter = 'nopert'

	def plot(self, data, avg_data, output_dir):
		# plot_data_all[instance_class][line][x]
		plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))

		fname_str = {'flowdec' : 'dim'}

		for attr in ['flowdec']:

			# Uncomment if interested
			# # By cut
			# for k, v in avg_data.iteritems():
			# 	if key_filters_setcover[attr](k):
			# 		instance_class = setcover_instance_name(k)
			# 		line = 'Width ' + str(key_selectors_setcover['width_factor'](k)) + '%'
			# 		ncuts = len(v['flowdec'][0])
			# 		for i in range(ncuts):
			# 			plot_data_all[instance_class][line][i+1] = v['flowdec'][0][i]
			# 		line = 'Feasible, Width ' + str(key_selectors_setcover['width_factor'](k)) + '%'
			# 		ncuts = len(v['flowdec_feas'][0])
			# 		for i in range(ncuts):
			# 			plot_data_all[instance_class][line][i+1] = v['flowdec_feas'][0][i]

			# for instance_class, plot_data in plot_data_all.iteritems():
			# 	plt.clf()
			# 	for i, k in enumerate(['Width 5%', 'Width 10%', 'Width 20%', 'Width 40%', 'Width 60%', 'Width 80%', 'Width 100%']):
			# 		plot_dict(plot_data[k], k, marker=markers[i], color=colors[i], linestyle='-')
			# 	for i, k in enumerate(['Feasible, Width 5%', 'Feasible, Width 10%', 'Feasible, Width 20%', 'Feasible, Width 40%', 'Feasible, Width 60%', 'Feasible, Width 80%', 'Feasible, Width 100%']):
			# 		plot_dict(plot_data[k], k, marker=markers[i], color=colors[i], linestyle='--')
			# 	plt.legend(loc=0, prop={'size':14})
			# 	plt.ylim(bottom=0)
			# 	plt.xlabel('Cut number')
			# 	plt.ylabel('Dimension')

			# 	lgd = plt.legend(loc='upper center', prop={'size':12}, bbox_to_anchor=(0.5, -0.12), ncol=3)
			# 	pylab.savefig(OUTPUT_DIR + '/' + instance_class + '_' + fname_str[attr] + '_ncuts.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')
			# 	pylab.savefig(OUTPUT_DIR + '/' + instance_class + '_' + fname_str[attr] + '_ncuts.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')


			# By width
			for k, v in avg_data.iteritems():
				if key_filters_setcover[attr](k):
					instance_class = setcover_instance_name(k)
					width_factor = key_selectors_setcover['width_factor'](k)
					plot_data_all[instance_class]['Feasible set, 1st cut'][width_factor] = v['flowdec_feas'][0][0]
					plot_data_all[instance_class]['Feasible set, 5th cut'][width_factor] = v['flowdec_feas'][0][4]
					plot_data_all[instance_class]['Feasible set, 10th cut'][width_factor] = v['flowdec_feas'][0][9]
					plot_data_all[instance_class]['Relaxation, 1st cut'][width_factor] = v['flowdec'][0][0]
					plot_data_all[instance_class]['Relaxation, 5th cut'][width_factor] = v['flowdec'][0][4]
					plot_data_all[instance_class]['Relaxation, 10th cut'][width_factor] = v['flowdec'][0][9]

			for instance_class, plot_data in plot_data_all.iteritems():
				plt.clf()

				plot_dict(plot_data['Feasible set, 1st cut'], 'Feasible set, 1st cut', marker=markers[0], color=colors[0], linestyle='-')
				plot_dict(plot_data['Relaxation, 1st cut'], 'Relaxation, 1st cut', marker=markers[0], color=colors[0], linestyle='--')
				plot_dict(plot_data['Feasible set, 5th cut'], 'Feasible set, 5th cut', marker=markers[1], color=colors[1], linestyle='-')
				plot_dict(plot_data['Relaxation, 5th cut'], 'Relaxation, 5th cut', marker=markers[1], color=colors[1], linestyle='--')
				plot_dict(plot_data['Feasible set, 10th cut'], 'Feasible set, 10th cut', marker=markers[2], color=colors[2], linestyle='-')
				plot_dict(plot_data['Relaxation, 10th cut'], 'Relaxation, 10th cut', marker=markers[2], color=colors[2], linestyle='--')

				plt.legend(loc=0, prop={'size':14})
				plt.ylim(bottom=0)
				plt.xlabel('Width as a percentage of exact width (%)')
				plt.ylabel('Dimension')

				lgd = plt.legend(loc='upper center', prop={'size':12}, bbox_to_anchor=(0.5, -0.12), ncol=3)
				pylab.savefig(OUTPUT_DIR + '/' + instance_class + '_' + fname_str[attr] + '_width.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')
				pylab.savefig(OUTPUT_DIR + '/' + instance_class + '_' + fname_str[attr] + '_width.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')
