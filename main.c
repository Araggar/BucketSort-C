#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <stdlib.h>

void print_array(const unsigned int*, int*);
int compare_double(const void *x, const void *y);
int compare_int(const void *x, const void *y);

int main(int argc, char** argv) {
	if (argc < 5) {
		printf("Usage: %s ARRAY_SIZE NUMBER_OF_BUCKETS NUMBER_OF_PROCESSES"
			"PRINT_ORIGINAL < ARRAY_TO_SORT", argv[0]);
		exit(1);
	}

	// Parameters

	const unsigned int ARRAY_SIZE = atoi(argv[1]);
	const unsigned int NUMBER_OF_BUCKETS = atoi(argv[2]);
	const unsigned int NUMBER_OF_PROCESSES = atoi(argv[3]);
	const unsigned int PRINT_ORIGINAL = atoi(argv[4]);

	int ORIGINAL[ARRAY_SIZE];

	double max_n = ARRAY_SIZE;
	double min_n = 0;

	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (scanf("%d", &ORIGINAL[i]) != 1) {
			printf("Error reading Array\n");
			exit(1);
		}
	}

	// Bucket Bounds

	int bucket_sizes[NUMBER_OF_BUCKETS];
	int bucket_std_size = ARRAY_SIZE / NUMBER_OF_BUCKETS;
	int extra = ARRAY_SIZE % NUMBER_OF_BUCKETS;

	for (int i = 0 ; i < NUMBER_OF_BUCKETS; i++) {
		if (extra) {
			bucket_sizes[i] = bucket_std_size + 1;
			--extra;
		} else {
			bucket_sizes[i] = bucket_std_size;
		}
	}

	// Print parameters

	printf("Array size : %u\n"
		"Number of buckets: %u\n"
		"Number of processes %u\n"
		"Min: %f || Max: %f\n",
		ARRAY_SIZE, NUMBER_OF_BUCKETS, NUMBER_OF_PROCESSES,
		min_n, max_n);


	// Bucket 





	if (PRINT_ORIGINAL) {
		print_array(&ARRAY_SIZE, ORIGINAL);
	}

	qsort(ORIGINAL, ARRAY_SIZE, sizeof(int), compare_int);

	print_array(&ARRAY_SIZE, ORIGINAL);

	// Find bucket bounds













	return 0;
}

void print_array(const unsigned int* size, int* array) {
	printf("(");
	for (int i = 0; i < *size; i++) {
		printf("%d, ", array[i]);
	}
	printf("\b\b)\n");
	return;
}

// Not working, don't know why yet
int compare_double(const void *x, const void *y) {
	double* _x = (double*) x;
	double* _y = (double*) y;
	if (*_x < *_y) return -1;
	else if (*_x > *_y) return 1;
	return 0;
}

int compare_int(const void *x, const void *y) {
	int* _x = (int*) x;
	int* _y = (int*) y;
	return *_x - *_y;
}