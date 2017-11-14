from timeit import timeit
from sys import argv, maxsize
from argparse import ArgumentParser
import time
import os.path
import random_array

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

	folder = str(int(time.time()))
	os.mkdir(folder)
	path = os.path.join(folder, 'results.txt')
	array_path = os.path.join(folder, '{}.in'.format(args.size))
	random_array.main(args.size, array_path)

	

	total = args.processes * args.buckets
	contador = 0
	best_time = maxsize

	with open(path, 'w') as results:
		for p in range (1, args.processes + 1):
			for b in range(1, args.buckets + 1):
				exec_str = "run('mpirun {} -n {} {} \
					{} 0 < {}', stdout=trash)".format(
					args.name, p, args.size, b, array_path)
				_time = timeit(exec_str, number=1, setup='import os; from subprocess import run; trash=open(os.devnull, "w")')
				if _time < best_time:
					best_time = _time
					best_pb = p, b
				results.write("N_Pro.:{} | N_Buck.:{} | time:{:0.8f} ms\n".format(p, b, _time*1000))
				contador += 100
				print("{:1.2f}%".format(contador/total), end="\r")

if __name__ == "__main__":
	main()