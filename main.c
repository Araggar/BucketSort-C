#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <stdlib.h>

#include "intlist.h"

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

	int max_n = ARRAY_SIZE;
	int min_n = 0;

	// Bucket Bounds (NON-INCLUSIVE)

	int bucket_bounds[NUMBER_OF_BUCKETS];
	int bucket_std_size = ARRAY_SIZE / NUMBER_OF_BUCKETS;
	int extra = ARRAY_SIZE % NUMBER_OF_BUCKETS;

	if (extra) {
			bucket_bounds[0] = bucket_std_size + 1;
			--extra;
		} else {
			bucket_bounds[0] = bucket_std_size;
		}

	for (int i = 1 ; i < NUMBER_OF_BUCKETS; i++) {
		if (extra) {
			bucket_bounds[i] = bucket_bounds[i-1] + bucket_std_size + 1;
			--extra;
		} else {
			bucket_bounds[i] = bucket_bounds[i-1] + bucket_std_size;
		}
	}

	//print_array(&NUMBER_OF_BUCKETS, bucket_bounds); //  DEBUG

	// Count bucket sizes and store the Original array

	int bucket_array_sizes[NUMBER_OF_BUCKETS];

	for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
		bucket_array_sizes[i] = 0;
	}

	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (scanf("%d", &ORIGINAL[i]) != 1) {
			printf("Error reading Array\n");
			exit(1);
		}
		for (int b = 0; b < NUMBER_OF_BUCKETS; b++) {
			if (ORIGINAL[i] < bucket_bounds[b]) {
				bucket_array_sizes[b]++;
				break;
			}
		}
	}

	//print_array(&NUMBER_OF_BUCKETS, bucket_array_sizes); //  DEBUG

	// Build & Fill the buckets

	List buckets[NUMBER_OF_BUCKETS];
	for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
		intlist_init(&buckets[i], bucket_array_sizes[i]);
	}

	for (int i = 0; i < ARRAY_SIZE; i++) {
		for (int b = 0; b < NUMBER_OF_BUCKETS; b++) {
			if (ORIGINAL[i] < bucket_bounds[b]) {
				intlist_append(&buckets[b], &ORIGINAL[i]);
				break;
			}
		}
	}





	for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
		print_array(&bucket_array_sizes[i], (buckets[i].int_list));  // DEBUG
	}

	// Print parameters

	printf("Array size : %u\n"
		"Number of buckets: %u\n"
		"Number of processes %u\n"
		"Min: %i || Max: %i\n",
		ARRAY_SIZE, NUMBER_OF_BUCKETS, NUMBER_OF_PROCESSES,
		min_n, max_n);

	// Original array 

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