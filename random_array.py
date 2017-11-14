from random import randrange
from sys import argv, exit

def main(size, name):
	with open("{}".format(name), 'w') as file:
		for _ in range(size):
			_to_write = str(randrange(size)) + " "
			file.write(_to_write)

	print("Done")

if __name__ == "__main__":
	if (len(argv) < 3) :
		print("Usage: {} ARRAY_SIZE FILE_NAME");
		exit(1)

	main(int(argv[1]), argv[2])