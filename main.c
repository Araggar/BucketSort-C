#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

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

int main(int argc, char** argv) {

	if (argc < 4) {
		printf("Usage: %s <ARRAY_SIZE> <NUMBER_OF_BUCKETS>"
			" <PRINT_ORIGINAL> <ARRAY_TO_SORT>", argv[0]);
		exit(1);
	}

	const unsigned int ARRAY_SIZE = atoi(argv[1]);
	const unsigned int NUMBER_OF_BUCKETS = atoi(argv[2]);
	const unsigned int PRINT_ORIGINAL = atoi(argv[3]);

	int max_n = ARRAY_SIZE;
	int min_n = 0;

	int rank, size, ierr;
	MPI_Status status;
	MPI_File array_file;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// processo 0 salva o array original e faz um broadcast para os outros processos
	// ------------------------------------------------------------------------

	int ORIGINAL[ARRAY_SIZE];
	if (rank == 0) {
		for (int i = 0; i < ARRAY_SIZE; i++) {
			if (scanf("%d", &ORIGINAL[i]) != 1) {
				printf("Error reading Array\n");
				exit(1);
			}
		}
	}

	MPI_Bcast(&ORIGINAL, ARRAY_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

	// Bucket Bounds (NON-INCLUSIVE)
	// ------------------------------------------------------------------------

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

    // Count bucket sizes and store the Original array
	// ------------------------------------------------------------------------

	int bucket_array_sizes[NUMBER_OF_BUCKETS];

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

	// Build & Fill the buckets
	// ------------------------------------------------------------------------

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

	// ------------------------------------------------------------------------
		
	int* bucket_tag_buffer;
	int bucket_id;
	unsigned int current_bucket = 0;
	unsigned int remaining_buckets = NUMBER_OF_BUCKETS;

	// ------------------------------------------------------------------------

	// processo mestre
	if (rank == 0) {

		// se tiver só um processo ou apenas um bucket
		if (size == 1 || NUMBER_OF_BUCKETS == 1) {
			for (int i = 0; i < NUMBER_OF_BUCKETS; i++) {
				if (bucket_array_sizes[i] > 1) {
					qsort(buckets[i].int_list, bucket_array_sizes[i], sizeof(int), compare_int);
				}
			}
			current_bucket = NUMBER_OF_BUCKETS;
			remaining_buckets = 0;

		// caso tenha mais processos e/ou mais buckets
		} else {

			// caso o numero de processo for maior que o de buckets
			if (size > NUMBER_OF_BUCKETS) {

				// envia os buckets iniciais
				for (int i = 1; i < NUMBER_OF_BUCKETS + 1; i++) {
					if (bucket_array_sizes[current_bucket] > 1) {
						MPI_Send(&current_bucket, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
					}
					current_bucket++;
				}

				// espera o retorno dos buckets ordenados
				while (remaining_buckets != 0) {
					MPI_Recv(&bucket_id, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
					MPI_Recv(buckets[bucket_id].int_list, bucket_array_sizes[bucket_id], MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					remaining_buckets--;
				}

			// caso tenha mais ou o mesmo numero de buckets igual ao de processos
			} else {

				// envia os buckets iniciais
				for (int i = 1; i < size ; i++) {
					if (bucket_array_sizes[current_bucket] > 1) {
						MPI_Send(&current_bucket, 1, MPI_INT, i, 0, MPI_COMM_WORLD); // envia id do bucket
					}
					current_bucket++;
				}

				// enquanto tiver buckets ele irá continuar recebendo eles ordenado, e enviando um novo
				while (remaining_buckets != 0 && current_bucket <= NUMBER_OF_BUCKETS) {

					// recebe o bucket do escravo
					MPI_Recv(&bucket_id, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
					MPI_Recv(buckets[bucket_id].int_list, bucket_array_sizes[bucket_id], MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					remaining_buckets--;

					//envia para esse mesmo escravo um novo bucket valido
					while (remaining_buckets != 0 && current_bucket <= NUMBER_OF_BUCKETS) {
						if (bucket_array_sizes[current_bucket] > 1) {
							MPI_Send(&current_bucket, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
							remaining_buckets--;
							current_bucket++;
							break;
						}
						current_bucket++;
						remaining_buckets--;
					}
				}
			}
		}

		// envia ID = -1, sinalizando pra os outros processo que não tem mais buckets.
		bucket_id = -1;
		for (int i = 1; i < size; i++) {
			MPI_Send(&bucket_id, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}

		// imprime o array original
		if (PRINT_ORIGINAL) {
			print_array(&ARRAY_SIZE, ORIGINAL);
		}

		// imprime o array ordenado
		for (int i = 0; i < NUMBER_OF_BUCKETS; i++)
			if (bucket_array_sizes[i] != 0)
				print_buckets(bucket_array_sizes[i], buckets[i].int_list);

		printf("\n");
	}

	// processos escravos
	else {

		while(1) {

			// recebe qual o ID do bucket a ser ordenado
			MPI_Recv(&current_bucket, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// caso o ID seja -1, o processo escravo sai do loop e finaliza
			if (current_bucket == -1) {
				break;
			}

			// ordena o bucket
			qsort(buckets[current_bucket].int_list, bucket_array_sizes[current_bucket], sizeof(int), compare_int); // sort o array

			// envia o ID do bucket ordenado e o próprio bucket.
			MPI_Send(&current_bucket, 1, MPI_INT, 0, 1, MPI_COMM_WORLD); // devolve o array sorted com a tag = bucket
			MPI_Send(buckets[current_bucket].int_list, bucket_array_sizes[current_bucket], MPI_INT, 0, 2, MPI_COMM_WORLD); // devolve o array sorted com a tag = bucket
		}
	}

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
