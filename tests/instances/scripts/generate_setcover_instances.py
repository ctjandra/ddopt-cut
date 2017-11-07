# Generate set cover instances with small bandwidth
# For bandwidth 40, also solves the problem and writes the optimal value to a file

import math
import random
import itertools
import networkx as nx
import matplotlib.pyplot as plt
import numpy as np
import cplex


def generate_set_cover_constraints(n, setsize, bandwidth):
	m = n - bandwidth + 1

	matrix = np.zeros((m, n))
	inds = [[] for i in range(m)]
	coeffs = [[] for i in range(m)]

	for i in range(m):
		selected = random.sample(xrange(bandwidth), setsize)
		for j in sorted(selected):
			matrix[i][j+i] = 1
			inds[i].append("x" + str(j+i)) # assume x0 to x(n-1)
			coeffs[i].append(1)

	rows = [cplex.SparsePair(inds[i], coeffs[i]) for i in range(m)]

	return matrix, rows


def write_set_cover_mps(matrix, rows, filepath, write_opt=False):
	m = matrix.shape[0]
	n = matrix.shape[1]

	c = cplex.Cplex()

	c.objective.set_sense(c.objective.sense.minimize)

	objective = [1] * n
	lb = [0] * n
	ub = [1] * n
	coltypes = ["B"] * n
	colnames = ["x" + str(i) for i in range(n)]
	c.variables.add(obj = objective, lb = lb, ub = ub, types = coltypes, names = colnames)

	rhs = [1] * m
	senses = ["G"] * m
	rownames = ["c" + str(i) for i in range(m)]
	c.linear_constraints.add(lin_expr = rows, senses = senses, rhs = rhs, names = rownames)

	if write_opt:
		c.solve()
		f = open(filepath + ".opt", 'w')
		f.write(str(int(c.solution.get_objective_value())) + '\n')
		f.close()

	print filepath + ' generated'

	c.write(filepath + ".mps")

def generate_instance(n, setsize, bandwidth, filepath, write_opt=False):
	matrix, rows = generate_set_cover_constraints(n, setsize, bandwidth)
	write_set_cover_mps(matrix, rows, filepath, write_opt)


n_instances = 16
setsize = 30

bandwidths_opt = [40] # bandwidths for which we solve using CPLEX and write the optimal solution
bandwidths_nonopt = [50, 60]

# # Uncomment this if you do not need the optimal values
# bandwidths_opt = []
# bandwidths_nonopt = [40, 50, 60]

bandwidths = [(bd, True) for bd in bandwidths_opt] + [(bd, False) for bd in bandwidths_nonopt]

for bandwidth, write_opt in bandwidths:
	for n in [250]:
		for inst in range(n_instances):
			filepath = "sc_n" + str(n) + "_ss" + str(setsize) + "_bw" + str(bandwidth) + "_" + str(inst)
			generate_instance(n, setsize, bandwidth, filepath, write_opt)
