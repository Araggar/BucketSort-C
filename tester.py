from timeit import timeit
from sys import argv, maxsize
from argparse import ArgumentParser

import time
import os.path
import random_array
import os

def main():

	parser = ArgumentParser(description='Timing test for mpirun BucketSort-C')
	parser.add_argument('--size',dest='size',
		help='Size of array to be sorted', required=True,
		type=int)
	parser.add_argument('--name',dest='name',
		help='Name of mpi program', required=True)
	parser.add_argument('--max_buckets',dest='buckets',
		help='Time BucketSort from 1 to N buckets', required=True,
		type=int)
	parser.add_argument('--max_processes',dest='processes',
		help='Time BucketSort from 1 to N processes', required=True,
		type=int)

	args = parser.parse_args()

	folder = os.path.join("tests",str(int(time.time())))
	os.makedirs(folder)
	path = os.path.join(folder, 'results.txt')
	array_path = os.path.join(folder, '{}.in'.format(args.size))
	random_array.main(args.size, array_path)

	

	total = args.processes * args.buckets
	contador = 0
	best_time = maxsize

	with open(path, 'w') as results:
		for p in range (1, args.processes + 1):
			for b in range(1, args.buckets + 1):
				exec_str = "call('mpirun -n {} {} {} {} 0 < {}', stdout=trash, shell=True)".format(
					p, args.name, args.size, b, array_path)
				_time = timeit(exec_str, number=1, setup='import os; from subprocess import call; trash=open(os.devnull, "w")')
				if _time < best_time:
					best_time = _time
					best_pb = p, b
				results.write("N_Pro.:{} | N_Buck.:{} | time:{:0.15f} s\n".format(p, b, _time))
				contador += 100
				print("{:1.2f}%".format(contador/total), end="\r")
		results.write("Best Processes/Buckets combo: P:{} B:{} with : {:0.15f} seconds".format(best_pb[0], best_pb[1], best_time))
		print("\n\nBest Processes/Buckets combo: P:{} B:{}, with {:0.15f} seconds".format(best_pb[0], best_pb[1], best_time))

if __name__ == "__main__":
	main()