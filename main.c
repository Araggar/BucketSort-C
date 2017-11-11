#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void print_array(const unsigned int*, double*);

int main(int argc, char** argv) {
	if (argc < 5) {
		printf("Usage: %s ARRAY_SIZE NUMBER_OF_BUCKETS NUMBER_OF_PROCESSES"
			"PRINT_ORIGINAL < ARRAY_TO_SORT", argv[0]);
		exit(1);
	}

	const unsigned int ARRAY_SIZE = atoi(argv[1]);
	const unsigned int NUMBER_OF_BUCKETS = atoi(argv[2]);
	const unsigned int NUMBER_OF_PROCESSES = atoi(argv[3]);
	const unsigned int PRINT_ORIGINAL = atoi(argv[4]);

	double ORIGINAL[ARRAY_SIZE];

	printf("Array size : %u\n"
		"Number of buckets: %u\n"
		"Number of processes %u\n",
		ARRAY_SIZE, NUMBER_OF_BUCKETS, NUMBER_OF_PROCESSES);

	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (scanf("%d", &ORIGINAL[i]) != 1) {
			printf("Error reading Array\n");
			exit(1);
		}
	}


	if (PRINT_ORIGINAL) {
		print_array(&ARRAY_SIZE, ORIGINAL);
	}











	return 0;
}

void print_array(const unsigned int* size, double* array) {
	printf("(");
	for (int i = 0; i < *size; i++) {
		printf("%d, ", array[i]);
	}
	printf("\b\b)\n");
	return;
}