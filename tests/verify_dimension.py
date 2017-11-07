# This script reads the output of a log file, in particular the points from flow decomposition, and prints a warning if
# they are not affinely independent.

import sys
import numpy

def print_rank(mtx, num):
	"""Print the dimension of the points and a warning if not full-dimensional. 
	num is just the iteration number printed in the output."""

	rank = numpy.linalg.matrix_rank(numpy.array(mtx))
	n = len(mtx)
	m = len(mtx[0])
	print sys.argv[1], num, " - Dimension: ", rank - 1, "/", n - 1, "(" + str(m-1) + ")"
	if rank != n:
		print "   **** Warning:", sys.argv[1], "- Not full-dimensional!"

if len(sys.argv) <= 1:
	print "Usage:", sys.argv[0], "[file-to-analyze]"
	sys.exit()

mtx = []
it = 0
f = open(sys.argv[1], 'r')
for line in f.readlines():

	# Start of a new set of points
	if line.startswith("Decomposition weights"):
		if len(mtx) > 0:
			it += 1
			print_rank(mtx, it)
			mtx = []

	# Collect points
	if line.startswith("Path"):
		x = map(int, line.split(':')[1].split())
		x.append(1) # to check affine independence instead of linear independence
		mtx.append(x)

f.close()
