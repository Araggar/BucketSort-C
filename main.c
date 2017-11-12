#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <mpi.h>

#include "intlist.h"

// Print parameters
void print_parameters(const unsigned int ARRAY_SIZE,
					  const unsigned int NUMBER_OF_BUCKETS,
					  int min_n, int max_n);

void print_buckets(const unsigned int size, int* array);

// Original array 
void print_array(const unsigned int*, int*);

int compare_double(const void *x, const void *y);

int compare_int(const void *x, const void *y);

// Bucket Bounds (NON-INCLUSIVE)
int *set_bounds(const unsigned int ARRAY_SIZE, 
				const unsigned int NUMBER_OF_BUCKETS);

// Count bucket sizes and store the Original array
int *set_bucket_sizes(int *bucket_bounds,
				      const unsigned int ARRAY_SIZE,
				      const unsigned int NUMBER_OF_BUCKETS,
				      int *ORIGINAL);

// Build & Fill the buckets
List *build_buckets(int *bucket_bounds, int *bucket_array_sizes,
				   const unsigned int ARRAY_SIZE, 
				   const unsigned int NUMBER_OF_BUCKETS, 
				   int *ORIGINAL);

int main(int argc, char** argv) {

	if (argc < 4) {
		printf("Usage: %s <ARRAY_SIZE> <NUMBER_OF_BUCKETS>"
			" <PRINT_ORIGINAL> <ARRAY_TO_SORT>", argv[0]);
		exit(1);
	}

	const unsigned int ARRAY_SIZE = atoi(argv[1]);
	const unsigned int NUMBER_OF_BUCKETS = atoi(argv[2]);
	const unsigned int PRINT_ORIGINAL = atoi(argv[3]);

	/*int ORIGINAL[ARRAY_SIZE];
	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (scanf("%d", &ORIGINAL[i]) != 1) {
			printf("Error reading Array\n");
			exit(1);
		}
	}
*/
	unsigned int current_bucket = 0;
	int max_n = ARRAY_SIZE;
	int min_n = 0;

	int rank, size, ierr;
	MPI_Status status;
	MPI_File array_file;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int ORIGINAL[ARRAY_SIZE];

	/*ierr = MPI_File_open(MPI_COMM_WORLD, argv[4], MPI_MODE_RDONLY, MPI_INFO_NULL, &array_file);
    if (ierr) {
        if (rank == 0) fprintf(stderr, "%s: Couldn't open file %s\n", argv[0], argv[1]);
        MPI_Finalize();
        exit(2);
    }*/

    // -------------------------------------------------------------------------------------

    /*int *ORIGINAL = calloc(ARRAY_SIZE, sizeof(int));
    ORIGINAL[0] = 1;
    ORIGINAL[1] = 4;
    ORIGINAL[2] = 4;
    ORIGINAL[3] = 6;
    ORIGINAL[4] = 1;
    ORIGINAL[5] = 2;
    ORIGINAL[6] = 3;*/

    int *bucket_bounds = set_bounds(ARRAY_SIZE, NUMBER_OF_BUCKETS);
		
	//print_array(&NUMBER_OF_BUCKETS, bucket_bounds); //  DEBUG

	int *bucket_array_sizes = 
	set_bucket_sizes(bucket_bounds, ARRAY_SIZE, NUMBER_OF_BUCKETS, ORIGINAL);

	//print_array(&NUMBER_OF_BUCKETS, bucket_array_sizes); //  DEBUG

	List *buckets = 
	build_buckets(bucket_bounds, bucket_array_sizes, ARRAY_SIZE, NUMBER_OF_BUCKETS, ORIGINAL);

	int* bucket_tag_buffer;
	int bucket_id;

	/*for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
		print_array(&bucket_array_sizes[i], (buckets[i].int_list));  // DEBUG
	}*/

	// -----------------------------------------------------------------------------------


	if (rank == 0) {

		/*MPI_Offset filesize;
		ORIGINAL = malloc(sizeof(int) * filesize);
		MPI_File_get_size(array_file, &filesize);
		MPI_File_read(array_file, ORIGINAL, filesize, MPI_INT, MPI_STATUS_IGNORE);*/
		for (int i = 0; i < ARRAY_SIZE; i++) {
			if (scanf("%d", &ORIGINAL[i]) != 1) {
				printf("Error reading Array\n");
				exit(1);
			}
		}

	MPI_Bcast(&ORIGINAL, ARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

		print_parameters(ARRAY_SIZE, NUMBER_OF_BUCKETS, min_n, max_n);

		// se tiver só um processo
		if (size == 1) {
			for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
				if (bucket_array_sizes[i] != 0) {
					qsort(buckets[i].int_list, bucket_array_sizes[i], sizeof(int), compare_int);
				}
			}
			current_bucket = NUMBER_OF_BUCKETS;
		}

		// send message

		// distribuindo um bucket para todos os processos
		for (int i = 1; i < size && (current_bucket != NUMBER_OF_BUCKETS) ; i++) {
			if (bucket_array_sizes[current_bucket]) {
				MPI_Send(&current_bucket, 1, MPI_INT, i, 0, MPI_COMM_WORLD); // envia id do bucket
				printf("Rank 0 sent a bucket to Rank %i\n", i);
			}
			current_bucket++;
		}

		while (current_bucket != NUMBER_OF_BUCKETS) {
			// recebe de um escravo

			
			MPI_Recv(&bucket_id, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
			MPI_Recv(&buckets[bucket_id].int_list, bucket_array_sizes[bucket_id], MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			//envia para esse mesmo escravo um novo bucket
			while (current_bucket < NUMBER_OF_BUCKETS) {
				if (bucket_array_sizes[current_bucket]) {
					MPI_Send(&current_bucket, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
				}
				current_bucket++;
			}
			current_bucket++;
			printf(" current_bucket %i\n", current_bucket);
		}

		// bcast
		bucket_id = -1;
		for (int i = 1; i < size; i++) {
			MPI_Send(&bucket_id, 1, MPI_INT, i, 0, MPI_COMM_WORLD); // sinaliza os processos que nao existem mais buckets
		}

		 

	/*	for (int i = 1; (i < size + 1) ; i++)
			MPI_Irecv(&buckets[i], bucket_array_sizes[i], MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);*/

		// MPI_Wait(&request, MPI_STATUS_IGNORE);

		if (PRINT_ORIGINAL) {
			print_array(&ARRAY_SIZE, ORIGINAL);
		}

		for (int i = 0; i < NUMBER_OF_BUCKETS; i++)
			print_buckets(bucket_array_sizes[i], buckets[i].int_list);

		//print_array(&ARRAY_SIZE, ORIGINAL);
	}

	// recebe os buckets e ordena ate receber bcast do 
	else {
		while(1) {
			MPI_Recv(&bucket_id, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // guarda o tamanho do array
			// break if tag = -1
			if (bucket_id == -1) {
				break;
			}
			qsort(buckets[bucket_id].int_list, bucket_array_sizes[bucket_id], sizeof(int), compare_int); // sort o array
			printf("Rank %i recv a bucket from Rank 0\n", rank);
			MPI_Send(&bucket_id, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); // devolve o array sorted com a tag = bucket
			MPI_Send(&ORIGINAL, bucket_id, MPI_INT, 0, 2, MPI_COMM_WORLD); // devolve o array sorted com a tag = bucket
		}
	}

	// MPI_File_close(&array_file);
	MPI_Finalize();

	return 0;
}

void print_parameters(const unsigned int ARRAY_SIZE,
					  const unsigned int NUMBER_OF_BUCKETS,
					  int min_n, int max_n) {
	printf("Array size : %u\n"
		"Number of buckets: %u\n"
		"Min: %i || Max: %i\n",
		ARRAY_SIZE, NUMBER_OF_BUCKETS,
		min_n, max_n);
}

void print_buckets(const unsigned int size, int* array) {
	for (int i = 0; i < size; i++) {
		printf("%d, ", array[i]);
	}
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

int *set_bounds(const unsigned int ARRAY_SIZE, 
				const unsigned int NUMBER_OF_BUCKETS) {

	int *bucket_bounds = malloc(sizeof(int) * NUMBER_OF_BUCKETS);

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
	return bucket_bounds;
}

int *set_bucket_sizes(int *bucket_bounds,
				      const unsigned int ARRAY_SIZE,
				      const unsigned int NUMBER_OF_BUCKETS,
				      int *ORIGINAL) {

	int *bucket_array_sizes = malloc(sizeof(int) * NUMBER_OF_BUCKETS);

	for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
		bucket_array_sizes[i] = 0;
	}
	for (int i = 0; i < ARRAY_SIZE; i++) {
		for (int b = 0; b < NUMBER_OF_BUCKETS; b++) {
			if (ORIGINAL[i] < bucket_bounds[b]) {
				bucket_array_sizes[b]++;
				break;
			}
		}
	}
	return bucket_array_sizes;
}

List *build_buckets(int *bucket_bounds, int *bucket_array_sizes,
				   const unsigned int ARRAY_SIZE, 
				   const unsigned int NUMBER_OF_BUCKETS, 
				   int *ORIGINAL) {

	List *buckets = malloc(sizeof(List) * NUMBER_OF_BUCKETS);

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
	return buckets;
}