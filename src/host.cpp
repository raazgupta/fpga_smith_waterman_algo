#include "xcl2.hpp"
#include <vector>

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

#define N 9
#define M 8

const short GAP_i = -5;
const short GAP_d = -5;
const short MATCH = 1;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;

// Computer matrices in Software

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
	for (i = 1; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

void fillQuery(char *query) {
    //query = '-CATTCAC';
    //strcpy(query,"CATTCAC");
    strcpy(query, "-CTCGCAGC");
}

void fillDatabase(char *database) {
	//database = '-CTCGCAGC';
	//strcpy(database, "CTCGCAGC");
	strcpy(database,"-CATTCAC");
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



int main(int argc, char** argv)
{
	printf("starting HOST code \n");
	fflush(stdout);

	// Allocate Memory in Host Memory
    // char *query = (char*) malloc(sizeof(char) * N);
    // char *database = (char*) malloc(sizeof(char) * M);
    //int *similarity_matrix = (int*) malloc(sizeof(int) * N * M);
    // short *direction_matrixhw = (short*) malloc(sizeof(short) * N * M);
    // int *max_index = (int *) malloc(sizeof(int));


    // std::vector<int,aligned_allocator<int>> source_in1(DATA_SIZE);
	// std::vector<int,aligned_allocator<int>> source_in2(DATA_SIZE);
	// std::vector<int,aligned_allocator<int>> source_hw_results(DATA_SIZE);
	// std::vector<int,aligned_allocator<int>> source_sw_results(DATA_SIZE);

	// Allocate Memory in Host Memory
	std::vector<char,aligned_allocator<char>> query(sizeof(char) * N);
	std::vector<char,aligned_allocator<char>> database(sizeof(char) * M);
	std::vector<int,aligned_allocator<int>> similarity_matrix(sizeof(int) * N * M);
	std::vector<short,aligned_allocator<short>> direction_matrixhw(sizeof(short) * N * M);
	std::vector<int,aligned_allocator<int>> max_index(sizeof(int));


	printf("array defined! \n");

	fflush(stdout);

    cl_int err;

    //fillRandom(query.data(), N);
    //fillRandom(database.data(), M);

    fillQuery(query.data());
    fillDatabase(database.data());

    memset(similarity_matrix.data(), 0, sizeof(int) * N * M);
    memset(direction_matrixhw.data(), 0, sizeof(short) * N * M);



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
    		sizeof(char) * N, query.data(), &err));
    OCL_CHECK(err, cl::Buffer input_database   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
    		sizeof(char) * M, database.data(), &err));
    OCL_CHECK(err, cl::Buffer output_similarity_matrix (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
    		sizeof(int) * M * N, similarity_matrix.data(), &err));
    OCL_CHECK(err, cl::Buffer output_direction_matrixhw (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
    		sizeof(short) * M * N, direction_matrixhw.data(), &err));
    OCL_CHECK(err, cl::Buffer output_max_index (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE,
        		sizeof(int), max_index.data(), &err));

    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({input_query, input_database, output_similarity_matrix, output_direction_matrixhw, output_max_index},0/* 0 means from host*/));

    OCL_CHECK(err, err = krnl_compute_matrices.setArg(0, input_query));
    OCL_CHECK(err, err = krnl_compute_matrices.setArg(1, input_database));
    OCL_CHECK(err, err = krnl_compute_matrices.setArg(2, output_max_index));
    OCL_CHECK(err, err = krnl_compute_matrices.setArg(3, output_similarity_matrix));
    OCL_CHECK(err, err = krnl_compute_matrices.setArg(4, output_direction_matrixhw));

    // Launch the Kernel
    // For HLS kernels global and local size is always (1,1,1). So, it is recommended
    // to always use enqueueTask() for invoking HLS kernel
    OCL_CHECK(err, err = q.enqueueTask(krnl_compute_matrices));

    // Copy Result from Device Global Memory to Host Local Memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({output_similarity_matrix, output_direction_matrixhw, output_max_index},CL_MIGRATE_MEM_OBJECT_HOST));
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
	}
	for(int i= 0; i <N*M; i++) {
		directionMatrixSW[i] = 0;
	}

	compute_matrices_sw(query.data(), database.data(),max_index_sw, matrix, directionMatrixSW );

	printf("both ended\n");


	for (int i = 0; i < N * M; i++) {
		//printf("SW[%d]: %d; HW[%d]: %d \n",i,directionMatrixSW[i],i,direction_matrixhw[i]);
		if (directionMatrixSW[i] != direction_matrixhw[i]) {
			printf("Error, mismatch in the results, i + %d, SW: %d, HW %d \n",
					i, directionMatrixSW[i], direction_matrixhw[i]);
			return EXIT_FAILURE;
		}
	}

	printf("computation ended!- RESULTS CORRECT \n");

}
