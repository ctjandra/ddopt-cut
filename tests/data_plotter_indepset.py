import collections
import matplotlib
import matplotlib.pyplot as plt
import pylab
import numpy

from settings import *



## Independent set: Solving time and number of nodes
# x-axis: Number of cuts
# y-axis: Solving time or Number of nodes
# Plot lines: Width

class PlotterIndepSetTimeNodes(object):

	instance_type = 'indepset'
	run_directory = 'indepset_main'
	required_fields = ['nodes', 'bddtime', 'cplextime', 'totaltime']
	filename_filter = '^((?!_lr|_objcut|_nocutsoc).)*$' # files that do not contain '_lr', '_objcut', '_nocutsoc'
	filename_filter_per_field = {'bddtime' : '^((?!noddcuts).)*$'} # files that do not contain 'noddcuts'

	def plot(self, data, avg_data, output_dir):
		for attr in ['nodes', 'totaltime']:

			# plot_data_all[instance_class][line][x]
			plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))

			# Extract data for ncuts >= 1
			for k, v in avg_data.iteritems():
				if key_filters_indepset['standard'](k):
					instance_class = indepset_instance_name(k)
					line = 'Width ' + str(key_selectors_indepset['width'](k))
					x_plot = key_selectors_indepset['ncuts'](k)
					plot_data_all[instance_class][line][x_plot] = v[attr][0]

			# Use nocuts data for x = 0 for all lines
			for k, v in avg_data.iteritems():
				if key_filters_indepset['standard_nocuts'](k):
					instance_class = indepset_instance_name(k)
					for line, pd in plot_data_all[instance_class].iteritems():
						pd[0] = v[attr][0]

			# Extract CPLEX without cuts and CPLEX with cuts separately
			nocutscplex_values = {}
			cutscplex_values = {}
			for k, v in avg_data.iteritems():
				if key_filters_indepset['standard_nocuts'](k):
					instance_class = indepset_instance_name(k)
					nocutscplex_values[instance_class] = v[attr][0]
				if key_filters_indepset['standard_nocuts_sc0'](k):
					instance_class = indepset_instance_name(k)
					cutscplex_values[instance_class] = v[attr][0]

			# Extract the union of the values of the xaxis, plus zero
			xaxis = [0] + sorted(list(set([key_selectors_indepset['ncuts'](k) for k in avg_data.keys() if key_filters_indepset['standard'](k)])))

			# Plot data
			for instance_class, plot_data in plot_data_all.iteritems():
				plt.clf()
				for i, (k, v) in enumerate(sorted(plot_data.iteritems(), key=lambda k: natural_sort_key(k[0]))):
					k = k.replace('Width 100000', 'Exact width')
					plot_dict(v, k, marker=markers[i], color=colors[i])
				plt.plot(xaxis, [nocutscplex_values[instance_class] for t in range(len(xaxis))],
					label="CPLEX without cuts", linestyle='--', color="#000000", linewidth=1.5, solid_capstyle="round")
				plt.plot(xaxis, [cutscplex_values[instance_class] for t in range(len(xaxis))],
					label="CPLEX with cuts", linestyle=':', color="#000000", linewidth=1.5, solid_capstyle="round")
				plt.legend(loc=0, prop={'size':14})
				plt.ylim(bottom=0)

				plt.xlim((-0.25, 10.25))

				# Specific cases:
				if attr == 'nodes':
					if instance_class == 'random_400_80':
						plt.ylim(bottom=0, top=19500)
						plt.yticks(numpy.arange(plt.ylim()[0], plt.ylim()[1], 2000))
						plt.legend(loc=9, prop={'size':14}, ncol=2)
					else:
						plt.ylim(bottom=0)
						plt.legend(loc=0, prop={'size':14})

				plt.xlabel(labels['ncuts'])
				plt.ylabel(labels[attr])
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '.pdf')
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '.svg')



## Independent set: Lagrangian cuts
# x-axis: Number of cuts
# y-axis: Attribute
# Plot lines: Width, Target vs Lagrangian

class PlotterIndepSetLagrangian(object):

	instance_type = 'indepset'
	run_directory = 'indepset_main'
	required_fields = ['nodes', 'bddtime', 'cplextime', 'totaltime']
	filename_filter = '400_80'

	def plot(self, data, avg_data, output_dir):
		for attr in ['nodes', 'totaltime']:

			# plot_data_all[instance_class][line][x]
			plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))

			# Extract data
			for k, v in avg_data.iteritems():
				if key_filters_indepset['objcut'](k):
					instance_class = indepset_instance_name(k)
					line = 'Target, Width ' + str(key_selectors_indepset['width'](k))
					line = line.replace('Width 100000', 'Exact width')
					x_plot = key_selectors_indepset['ncuts'](k)
					plot_data_all[instance_class][line][x_plot] = v[attr][0]
				if key_filters_indepset['lroc'](k):
					instance_class = indepset_instance_name(k)
					line = 'Lagrangian, Width ' + str(key_selectors_indepset['width'](k))
					line = line.replace('Width 100000', 'Exact width')
					x_plot = key_selectors_indepset['ncuts'](k)
					plot_data_all[instance_class][line][x_plot] = v[attr][0]

			# Use nocuts data for x = 0 for all lines (only objective bound)
			for k, v in avg_data.iteritems():
				if key_filters_indepset['nocutsoc'](k):
					instance_class = indepset_instance_name(k)
					line = 'Target, Width ' + str(key_selectors_indepset['width'](k))
					line = line.replace('Width 100000', 'Exact width')
					plot_data_all[instance_class][line][0] = v[attr][0]
					line = 'Lagrangian, Width ' + str(key_selectors_indepset['width'](k))
					line = line.replace('Width 100000', 'Exact width')
					plot_data_all[instance_class][line][0] = v[attr][0]

			# Extract CPLEX without cuts and CPLEX with cuts separately
			nocutscplex_values = {}
			cutscplex_values = {}
			for k, v in avg_data.iteritems():
				if key_filters_indepset['standard_nocuts'](k):
					instance_class = indepset_instance_name(k)
					nocutscplex_values[instance_class] = v[attr][0]
				if key_filters_indepset['standard_nocuts_sc0'](k):
					instance_class = indepset_instance_name(k)
					cutscplex_values[instance_class] = v[attr][0]

			# Extract the union of the values of the xaxis, plus zero
			xaxis = [0] + sorted(list(set([key_selectors_indepset['ncuts'](k) for k in avg_data.keys() if key_filters_indepset['objcut'](k)])))

			# Plot data
			for instance_class, plot_data in plot_data_all.iteritems():
				plt.clf()

				i_lr = 0
				i_tg = 0
				ordered_keys = ['Lagrangian, Width 100', 'Target, Width 100', 'Lagrangian, Width 200', 'Target, Width 200',
					'Lagrangian, Width 400', 'Target, Width 400', 'Lagrangian, Width 600', 'Target, Width 600', 'Lagrangian, Exact width',
					'Target, Exact width'] # specific legend order for plotting
				for k in ordered_keys:
					v = plot_data[k]
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

				plt.xlim((-0.25, 10.25))

				lgd = plt.legend(loc='upper center', prop={'size':12}, bbox_to_anchor=(0.5, -0.12), ncol=3)
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '_lr.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')
				pylab.savefig(output_dir + '/' + instance_class + '_' + attr + '_lr.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')



## Independent set: Gap closed
# x-axis: Number of cuts
# y-axis: Gap closed
# Plot lines: Width factor

class PlotterIndepSetGap(object):

	instance_type = 'indepset'
	run_directory = 'indepset_gap'
	required_fields = ['allgapclosed']

	def plot(self, data, avg_data, output_dir):
		# plot_data_all[instance_class][line][x]
		plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))
		nocuts_data = {}

		for k, v in avg_data.iteritems():
			if key_filters_indepset['gap_runs'](k):
				instance_class = indepset_instance_name(k)
				line = 'Width ' + str(key_selectors_indepset['width_factor'](k)) + '%'
				ncuts = len(v['allgapclosed'][0])

				plot_data_all[instance_class][line][0] = 0
				for i in range(ncuts):
					plot_data_all[instance_class][line][i+1] = v['allgapclosed'][0][i] * 100

			if key_filters_indepset['standard_nocuts_sc0'](k):
				instance_class = indepset_instance_name(k)
				ncuts = len(v['allgapclosed'][0])

				plot_data_all[instance_class]['CPLEX cuts'][0] = v['allgapclosed'][0][0] * 100
				for i in range(ncuts):
					plot_data_all[instance_class]['CPLEX cuts'][i+1] = v['allgapclosed'][0][i] * 100

		for instance_class, plot_data in plot_data_all.iteritems():
			plt.clf()
			for i, k in enumerate(['Width 1%', 'Width 5%', 'Width 10%', 'Width 20%', 'Width 40%', 'Width 60%', 'Width 80%', 'Width 100%']):
				plot_dict(plot_data[k], k, marker=markers[i], color=colors[i])

			plot_dict(plot_data['CPLEX cuts'], 'CPLEX cuts', marker=None, linestyle='--', color='#000000', linewidth=1)

			plt.legend(loc=0, prop={'size':14})
			plt.ylim(bottom=0, top=100)
			plt.xlabel(labels['ncuts'])
			plt.ylabel(labels['gap'])
			plt.xlim((-0.25, 30.25))

			lgd = plt.legend(loc='upper center', prop={'size':12}, bbox_to_anchor=(0.5, -0.12), ncol=3)
			pylab.savefig(output_dir + '/' + instance_class + '_gapclosed.pdf', bbox_extra_artists=(lgd,), bbox_inches='tight')
			pylab.savefig(output_dir + '/' + instance_class + '_gapclosed.svg', bbox_extra_artists=(lgd,), bbox_inches='tight')



## Independent set: Face dimensions
# x-axis: Cut number
# y-axis: Face dimension
# Plot lines: Width factor

class PlotterIndepSetDimensions(object):

	instance_type = 'indepset'
	run_directory = 'indepset_dim'
	required_fields = ['flowdec', 'flowdec_feas']


	def plot(self, data, avg_data, output_dir):
		# plot_data_all[instance_class][line][x]
		plot_data_all = collections.defaultdict(lambda: collections.defaultdict(dict))

		fname_str = {'flowdec' : 'dim', 'flowdec_pert' : 'dim_pert'}

		for attr in ['flowdec', 'flowdec_pert']:

			# Uncomment if interested
			# # By cut
			# for k, v in avg_data.iteritems():
			# 	if key_filters_indepset[attr](k):
			# 		instance_class = indepset_instance_name(k)
			# 		line = 'Width ' + str(key_selectors_indepset['width_factor'](k)) + '%'
			# 		ncuts = len(v['flowdec'][0])
			# 		for i in range(ncuts):
			# 			plot_data_all[instance_class][line][i+1] = v['flowdec'][0][i]
			# 		line = 'Feasible, Width ' + str(key_selectors_indepset['width_factor'](k)) + '%'
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
				if key_filters_indepset[attr](k):
					instance_class = indepset_instance_name(k)
					width_factor = key_selectors_indepset['width_factor'](k)
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


## Independent set: Time breakdown
# x-axis: Width
# y-axis: Solving time
# Plot lines: Breakdown in parts

class PlotterIndepSetTimeBreakdown(object):

	instance_type = 'indepset'
	run_directory = 'indepset_main'
	required_fields = ['bbtime', 'bddtime', 'firstcuttime', 'cplextime', 'totaltime']
	filename_filter = '^((?!_lr|_objcut|_nocutsoc).)*$' # files that do not contain '_lr', '_objcut', '_nocutsoc'


	def plot(self, data, avg_data, output_dir):
		for ncuts_plot in [1]:

			# plot_data_all[instance_class][line][x]
			plot_data_all = collections.defaultdict(lambda: collections.OrderedDict())

			# Extract data
			for k, v in avg_data.iteritems():
				if key_filters_indepset['standard'](k) and key_selectors_indepset['ncuts'](k) == ncuts_plot and 'firstcuttime' in v:
					instance_class = indepset_instance_name(k)
					x_plot = key_selectors_indepset['width'](k)
					if x_plot == 100000:
						x_plot = 800 # Handling exact width

					if "Branch-and-bound" not in plot_data_all[instance_class]: # initialize dicts
						plot_data_all[instance_class]["Branch-and-bound"] = {}
						plot_data_all[instance_class]["Above + Cut generation"] = {}
						plot_data_all[instance_class]["Above + LP at root"] = {}
						plot_data_all[instance_class]["Above + BDD construction"] = {}

					plot_data_all[instance_class]["Branch-and-bound"][x_plot] = v['bbtime'][0]
					plot_data_all[instance_class]["Above + Cut generation"][x_plot] = v['bbtime'][0] + v['firstcuttime'][0]
					plot_data_all[instance_class]["Above + LP at root"][x_plot] = v['cplextime'][0]
					plot_data_all[instance_class]["Above + BDD construction"][x_plot] = v['totaltime'][0]

			# Plot data
			for instance_class, plot_data in plot_data_all.iteritems():
				plt.clf()
				for i, (k, v) in enumerate(plot_data.iteritems()):
					plot_dict(v, k, marker=['x', 's', 'o', '^'][i], color=colors[i])
				plt.legend(loc=0, prop={'size':14})
				plt.ylim(bottom=0)
				plt.xlabel(labels['width'])
				plt.ylabel(labels['time'])
				pylab.savefig(OUTPUT_DIR + '/' + instance_class + '_timebreakdown_' + str(ncuts_plot) + '.pdf')
				pylab.savefig(OUTPUT_DIR + '/' + instance_class + '_timebreakdown_' + str(ncuts_plot) + '.svg')
