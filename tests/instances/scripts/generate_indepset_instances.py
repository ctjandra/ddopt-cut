# Generate random graphs for independent set

import random

def generate_random_graph(fname, n, p):
	f = open(fname, 'w')
	f.write('c "random instance"\n')

	edges = []
	for i in range(n):
		for j in range(i+1, n):
			if random.uniform(0, 1) < p:
				edges.append((i,j))

	f.write('p edges ' + str(n) + ' ' + str(len(edges)) + '\n')
	for e in edges:
		f.write('e ' + str(e[0]+1) + ' ' + str(e[1]+1) + '\n')


for n, p in [(400, 0.8), (300, 0.8), (250, 0.5), (120, 0.5)]:
	for i in range(10):
		fname = 'random_' + str(n) + '_' + str(int(p*100)) + '_' + str(i) + '.clq'
		generate_random_graph(fname, n, p)
