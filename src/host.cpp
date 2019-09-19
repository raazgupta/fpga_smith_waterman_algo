#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include <CL/cl_ext.h>
#include "xcl2.hpp"

#define N 64
#define M 128

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;

////////////////////////////////////////////////////////////////////////////////

short get(char data[], int key) {
				const int position = (key % 4) * 2;
				key /= 4;
				char mask = 0;
				mask &= 00000000;

				mask |= 3 << position;

				char fin_mask =0;
				fin_mask&= 00000000;

				fin_mask |= 3 << 0;
				return (((data[key] & mask) >> ( position)) & fin_mask);

			}


unsigned short* order_matrix_blocks(unsigned short* in_matrix){
	unsigned short * out_matrix = (unsigned short*)malloc(sizeof(unsigned short) * N * M);

	int num_diag = 0;
	int store_elem = 1;
	int store_index;
	int tmp_i = 0;
	int tmp_j = 0;
	int i = 0;
	int j;

	for(i = 0; i<(M + N - 1) * N;){
	//while(store_elem != 0){
		if(num_diag < M){
			tmp_j = num_diag;
			tmp_i = 0;
		}else{
			tmp_i = (num_diag + 1) - M;
			tmp_j = M - 1;
		}

		for(j = 0; j < store_elem; j++){
			store_index = tmp_j * N + tmp_i;
			out_matrix[store_index] = in_matrix[i];
			//printf("stored %d in index %d \n", out_matrix[store_index], store_index);
			tmp_j--;
			tmp_i++;
			i++;
		}
		num_diag++;
		if(num_diag >= M){
			store_elem--;
			i = num_diag * N + N - store_elem;
		}else{
			if(store_elem != N){
				store_elem++;
				i = num_diag * N;
			}
		}
	}


	return out_matrix;
}

void compute_matrices_sw_2(char *query, char *database,
		int *max_index, int *similarity_matrix, short *direction_matrix){
	int north = 0;
	      int west = 0;
	      int northwest = 0;


	      int maxValue = 0;
	      int localMaxIndex = 0;

	      int val = 0;
	      short dir = 0;
	      int maxIndexSw = 0;


	  //calculate my own SW
	  for(int i = 0; i < N * M; i++){
	    val = 0;
	    dir=CENTER;
	    if( i == 0){
	      north = 0;
	      northwest = 0;
	      west = 0;
	    }else if (i / N == 0){ //first row
	      north = 0;
	      northwest = 0;
	      west = similarity_matrix[i - 1];
	    }else if(i % N == 0){ // first col
	      west = 0;
	      northwest = 0;
	      north = similarity_matrix[i - N];
	    }else{
	      west = similarity_matrix[i - 1];
	      north = similarity_matrix[i - N];
	      northwest = similarity_matrix [i - N - 1];
	    }


	    //all set, compute
	    int jj = i / N;
	    int ii = i % N;

	    const short match = (query[ii] == database[jj]) ?  MATCH : MISS_MATCH;
	    int val1 = northwest + match;


	    if (val1 > val) {
	      val = val1;
	        dir = NORTH_WEST;
	    }
	    val1 = north + GAP_d;
	    if (val1 > val) {
	        val = val1;
	        dir = NORTH;
	    }
	    val1 = west + GAP_i;
	    if (val1 > val) {
	      val = val1;
	        dir = WEST;
	    }
	    //printf("val %d \n", val);
	    similarity_matrix[i] = val;
	    direction_matrix[i] = dir;

	    if(val > maxValue){
	      maxValue = val;
	      maxIndexSw = i;
	    }

	  }
}


void compute_matrices_sw(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix)
{
	//here the real computation starts...
	int index = 0;
	int i = 0;
	int j = 0;
	short dir = CENTER;
	short match = 0;
	int val = 0;
	int north = 0;
	int west = 0;
	int northwest = 0;
	int max_value = 0;
	int test_val = 0;

	max_index[0] = 0;

	for(index = N; index < N*M; index++) {
		dir = CENTER;
		val = 0;

		i = index % N; // column index
		j = index / N; // row index

		if(i == 0) {
			// first column
			west = 0;
			northwest = 0;
		} else {

			// all columns but first
			north = similarity_matrix[index - N];
			match = ( string1[i] == string2[j] ) ? MATCH : MISS_MATCH;

			test_val = northwest + match;
			if(test_val > val){
				val = test_val;
				dir = NORTH_WEST;
			}

			test_val = north + GAP_d;
			if(test_val > val){
				val = test_val;
				dir = NORTH;
			}

			test_val = west + GAP_i;
			if(test_val > val){
				val = test_val;
				dir = WEST;
			}

			similarity_matrix[index] = val;
			direction_matrix[index] = dir;
			west = val;
			northwest = north;
			if(val > max_value) {
				max_index[0] = index;
				max_value = val;
			}
		}
	}
}

/*
 Given an event, this function returns the kernel execution time in ms
 */
float getTimeDifference(cl_event event) {
	cl_ulong time_start = 0;
	cl_ulong time_end = 0;
	float total_time = 0.0f;

	clGetEventProfilingInfo(event,
	CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start,
	NULL);
	clGetEventProfilingInfo(event,
	CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end,
	NULL);
	total_time = time_end - time_start;
	return total_time / 1000000.0; // To convert nanoseconds to milliseconds
}


/*
 return a random number between 0 and limit inclusive.
 */
int rand_lim(int limit) {

	int divisor = RAND_MAX / (limit + 1);
	int retval;

	do {
		retval = rand() / divisor;
	} while (retval > limit);

	return retval;
}

/*
 Fill the string with random values
 */
void fillRandom(char* string, int dimension) {
	//fill the string with random letters..
	static const char possibleLetters[] = "ATCG";

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

int load_file_to_memory(const char *filename, char **result) {
	int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		*result = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *) malloc(size + 1);
	if (size != fread(*result, sizeof(char), size, f)) {
		free(*result);
		return -2; // -2 means file reading fail
	}
	fclose(f);
	(*result)[size] = 0;
	return size;
}

int main(int argc, char** argv) {
	printf("starting HOST code \n");
	fflush(stdout);
	int err;                            // error code returned from api calls

	char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * M);
	char *databasehw = (char*) malloc(sizeof(char) * (M + 2 *(N-1)));
//	int *similarity_matrix = (int*) malloc(sizeof(int) * N * M);
	char *direction_matrixhw = (char*) malloc(sizeof(char) * 256 * (N + M - 1)); //512 bits..
//	int *max_index = (int *) malloc(sizeof(int));

	printf("array defined! \n");

	fflush(stdout);

	fillRandom(query, N);
	fillRandom(database, M);
	fillRandom(databasehw, M+2*(N-1));

	for(int i = 0; i < N - 1 ; i ++)
		databasehw[i] = 'P';

	memcpy((databasehw + N - 1), database, M);

//	memset(similarity_matrix, 0, sizeof(int) * N * M);
	memset(direction_matrixhw, 0, sizeof(char) * 256 * (N + M - 1));

	// OPENCL HOST CODE AREA START
	    // get_xil_devices() is a utility API which will find the xilinx
	    // platforms and will return list of devices connected to Xilinx platform
	    std::vector<cl::Device> devices = xcl::get_xil_devices();
	    cl::Device device = devices[0];

	    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
	    OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
	    OCL_CHECK(err, std::string device_name = device.getInfo<CL_DEVICE_NAME>(&err));

	    // find_binary_file() is a utility API which will search the xclbin file for
	    // targeted mode (sw_emu/hw_emu/hw) and for targeted platforms.
	    std::string binaryFile = xcl::find_binary_file(device_name,"compute_matrices");

	    // import_binary_file() ia a utility API which will load the binaryFile
	    // and will return Binaries.
	    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);
	    devices.resize(1);
	    OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));
	    OCL_CHECK(err, cl::Kernel krnl_compute_matrices(program,"compute_matrices", &err));

	    // Allocate Buffer in Global Memory
	    // Buffers are allocated using CL_MEM_USE_HOST_PTR for efficient memory and
	    // Device-to-host communication
	    OCL_CHECK(err, cl::Buffer input_query   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
	    		sizeof(char) * N, query, &err));
	    OCL_CHECK(err, cl::Buffer input_database   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
	    		sizeof(char) * (M + 2 * (N - 1)), databasehw, &err));
	    OCL_CHECK(err, cl::Buffer output_direction_matrixhw (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
	    		sizeof(char) * 256 * (N + M - 1), direction_matrixhw, &err));

	    // Copy input data to device global memory
	    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({input_query, input_database, output_direction_matrixhw},0/* 0 means from host*/));

	    OCL_CHECK(err, err = krnl_compute_matrices.setArg(0, input_query));
	    OCL_CHECK(err, err = krnl_compute_matrices.setArg(1, input_database));
	    OCL_CHECK(err, err = krnl_compute_matrices.setArg(2, output_direction_matrixhw));

	    // Launch the Kernel
	    // For HLS kernels global and local size is always (1,1,1). So, it is recommended
	    // to always use enqueueTask() for invoking HLS kernel
	    OCL_CHECK(err, err = q.enqueueTask(krnl_compute_matrices));

	    // Copy Result from Device Global Memory to Host Local Memory
	    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({output_direction_matrixhw},CL_MIGRATE_MEM_OBJECT_HOST));
	    q.finish();
	// OPENCL HOST CODE AREA END

	// Software calculation
	int * matrix = ( int *) malloc(
			sizeof( int) * N * M);
	short * directionMatrixSW = ( short*) malloc(
			sizeof( short) * N * M);

	int * max_index_sw = ( int *) malloc(
				sizeof( int));

	for(int i = 0; i < N*M; i++){
		matrix[i] = 0;
		directionMatrixSW[i] = 0;
	}
	compute_matrices_sw_2(query, database,max_index_sw, matrix, directionMatrixSW );

	unsigned short * ordered_direction_matrix = (unsigned short *)malloc(sizeof(unsigned short) * N * M);

	for(int i = 0; i < N; i ++)
		printf("%c ", query[i]);
	printf("\n");

	for(int i = 0; i < M; i ++)
		printf("%c ", database[i]);
	printf("\n");
	for(int i = 0; i < M + 2 * (N -1); i ++)
			printf("%c ", databasehw[i]);
		printf("\n");

	for(int i = 0; i < N*M; i++){
		if(i % N == 0)
			printf("\n");
		printf(" %d ", directionMatrixSW[i]);
	}

	printf("hw version \n");

	for(int i = 0; i < N*(N + M - 1); i++){
		if ( i % N == 0)
			printf("\n");
		printf(" %d ", direction_matrixhw[i]);
	}

	int temp_index = 0;
	int iter = 1;
	//unsigned short tempMatrixBis[N * (N+M-1)];
	unsigned short *tempMatrixBis = (unsigned short*)malloc(sizeof(unsigned short) * (N*(N+M-1)));
	for(int i = 0; i < N * (N + M - 1); i++){
		tempMatrixBis[i] = get(direction_matrixhw, temp_index);
		temp_index++;
		if(temp_index % N == 0){
			temp_index = iter * 256;
			iter++;
		}
	}

	ordered_direction_matrix = order_matrix_blocks(tempMatrixBis);

	printf("hw version \n");

		for(int i = 0; i < N*M; i++){
			if ( i % N == 0)
				printf("\n");
			printf(" %d ", ordered_direction_matrix[i]);
		}

	printf("both ended\n");

	for (int i = 0; i < N * M; i++) {
		if (directionMatrixSW[i] != ordered_direction_matrix[i]) {
			printf("Error, mismatch in the results, i + %d, SW: %d, HW %d \n",
					i, directionMatrixSW[i], ordered_direction_matrix[i]);
			return EXIT_FAILURE;
		}
	}

	printf("computation ended!- RESULTS CORRECT \n");



	free(matrix);
	free(directionMatrixSW);
	free(databasehw);
//	free(max_index_sw);
	free(ordered_direction_matrix);

	return EXIT_SUCCESS;
}
