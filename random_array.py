from random import randrange
from sys import argv, exit

if (len(argv) < 3) :
	print("Usage: {} ARRAY_SIZE FILE_NAME");

with open("{}.txt".format(argv[2]), 'w') as file:
	size = int(argv[1])
	for _ in range(int(size)):
		_to_write = str(randrange(size)) + " "
		file.write(_to_write)

print("Done")