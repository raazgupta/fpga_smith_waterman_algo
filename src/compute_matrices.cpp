#include <string.h>
#include <ap_int.h>


// directions codes
static const int CENTER = 0;
static const int NORTH = 1;
static const int NORTH_WEST = 2;
static const int WEST = 3;

// scores used for Smith Waterman similarity computation
static const short GAP_i = -1;
static const short GAP_d = -1;
static const short MATCH = 2;
static const short MISS_MATCH = -1;


#define N 64
#define NUM_ELEM 64
#define M 128
#define DATABASE_SIZE M + 2 * (N - 1)
#define DIRECTION_MATRIX_SIZE (N + M - 1) * N
#define SIMILARITY_MATRIX_SIZE (N+M-1) * 2
#define MATRIX_SIZE N * M

extern "C" {

void update_database(ap_uint<512> *string2, ap_uint<512> *shift_db, int num_diagonals){
	int startingIndex = N + num_diagonals;
	update_database:for(int i=1; i<N; i++){
#pragma HLS PIPELINE
		shift_db[(i-1)/NUM_ELEM].range(((i-1)%NUM_ELEM)*2+1, ((i-1)%NUM_ELEM)*2) = shift_db[i/NUM_ELEM].range((i%NUM_ELEM)*2+1, (i%NUM_ELEM)*2);
	}
	shift_db[(N-1)/NUM_ELEM].range(((N-1)%NUM_ELEM) * 2 + 1,((N-1)%NUM_ELEM) * 2) = string2[startingIndex/NUM_ELEM].range((startingIndex%NUM_ELEM) * 2 +1, (startingIndex%NUM_ELEM) *2);
}

void store_diagonal(int directions_index, ap_uint<512> *direction_matrix_g, ap_uint<512> compressed_diag[1], int *similarity_matrix, int *similarityDiagonal) {

	memcpy(direction_matrix_g + directions_index, compressed_diag, sizeof(ap_uint<512>));

	int max_val = 0;
	int max_index = 0;

	find_simDiag_max_for: for(int i=N-1; i>=0; i--){
		if(similarityDiagonal[i]>max_val){
			max_val = similarityDiagonal[i];
			max_index = i;
		}
	}

	//Store max_index and max_value in similarity_matrix
	similarity_matrix[directions_index*2] = max_index;
	similarity_matrix[directions_index*2+1] = max_val;

	/*
	store_in_simMat_for: for(int i=0; i<N; i++){
		similarity_matrix[directions_index*N+i] = similarityDiagonal[i];
	}
	*/

}



void calculate_diagonal(int num_diagonals, ap_uint<512> string1[N/NUM_ELEM+1], int northwest[N + 1], int north[N + 1], int west[N + 1], int directions_index, ap_uint<512> compressed_diag[1], int similarityDiagonal[N], ap_uint<512> *shift_db){

	int databaseLocalIndex = 0;
	int from, to;
	from = N * 2 - 2;
	to = N * 2 - 1;

	//int loop_column_index = num_diagonals * 2;
	//int loop_max_value_index = num_diagonals * 2 + 1;

	calculate_diagonal_for: for(int index = N - 1; index >= 0; index --){
		int val = 0;

		const short q = string1[index/NUM_ELEM].range((index%NUM_ELEM)*2+1, (index%NUM_ELEM)*2);
		short db = shift_db[databaseLocalIndex/NUM_ELEM].range((databaseLocalIndex%NUM_ELEM)*2+1, (databaseLocalIndex%NUM_ELEM)*2);

		if(databaseLocalIndex < N-1) db = 9;

//		if(num_diagonals < N - 1 && databaseLocalIndex < N - 1 - num_diagonals) db = 9;

		const short match = (q == db) ? MATCH : MISS_MATCH;
		const short val1 = northwest[index] + match;
		const short val2 = north[index] + GAP_d;
		const short val3 = west[index] + GAP_i;

		if(val1 > val && val1 >= val2 && val1 >= val3){
			//val1
			northwest[index + 1] = north[index];
			north[index] = val1;
			west[index + 1] = val1;
			compressed_diag[0].range(to,from) = NORTH_WEST;
//			directionDiagonal[index] = NORTH_WEST;
			val = val1;
		} else if (val2 > val && val2 >= val3) {
			//val2
			northwest[index + 1] = north[index];
			north[index] = val2;
			west[index + 1] = val2;
			compressed_diag[0].range(to,from) = NORTH;
//			directionDiagonal[index] = NORTH;
			val = val2;
		}else if (val3 > val){
			//val3
			northwest[index + 1] = north[index];
			north[index] = val3;
			west[index + 1] = val3;
			compressed_diag[0].range(to,from) = WEST;
//			directionDiagonal[index] = WEST;
			val = val3;
		}else{
			//val
			northwest[index + 1] = north[index];
			north[index] = val;
			west[index + 1] = val;
			compressed_diag[0].range(to,from) = CENTER;
//			directionDiagonal[index] = CENTER;
		}

		similarityDiagonal[index] = val;

		databaseLocalIndex ++;
		from -= 2;
		to -= 2;
	}

}



void compute_matrices(ap_uint<512> *string1_g, ap_uint<512> *string2_g, ap_uint<512> *direction_matrix_g, int *max_index_value)
{
#pragma HLS INTERFACE m_axi port=string1_g offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=string2_g offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=direction_matrix_g offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=max_index_value offset=slave bundle=gmem2
//#pragma HLS INTERFACE m_axi port=similarity_matrix offset=slave bundle=gmem0


#pragma HLS INTERFACE s_axilite port=string1_g bundle=control
#pragma HLS INTERFACE s_axilite port=string2_g bundle=control
#pragma HLS INTERFACE s_axilite port=direction_matrix_g bundle=control
#pragma HLS INTERFACE s_axilite port=max_index_value bundle=control
//#pragma HLS INTERFACE s_axilite port=similarity_matrix bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control

	ap_uint<512> string1[N/NUM_ELEM + 1];
	ap_uint<512> string2[DATABASE_SIZE/NUM_ELEM + 1];


//	short direction_matrix[DIRECTION_MATRIX_SIZE];
//#pragma HLS ARRAY_PARTITION variable=direction_matrix complete dim=1

	memcpy(string1, string1_g, (N/NUM_ELEM+1) * 64);
	memcpy(string2, string2_g, (DATABASE_SIZE/NUM_ELEM+1) * 64);

	int north[N+1];
#pragma HLS ARRAY_PARTITION variable=north complete dim=1
	int west[N+1];
#pragma HLS ARRAY_PARTITION variable=west complete dim=1
	int northwest[N+1];
#pragma HLS ARRAY_PARTITION variable=northwest complete dim=1


/*
	short directionDiagonal[N];
#pragma HLS ARRAY_PARTITION variable=directionDiagonal complete dim=1
*/

	ap_uint<512> compressed_diag[1];
	ap_uint<512> shift_db[N/NUM_ELEM];

	init_dep_for:for(int i = 0; i <= N; i++){
		north[i] = 0;
		west[i] = 0;
		northwest[i] = 0;
	}

	init_db:for(int i = 0; i < N/NUM_ELEM; i++){
	#pragma HLS PIPELINE
			shift_db[i] = string2[i];
	}

	int directions_index = 0;


	int similarity_matrix[SIMILARITY_MATRIX_SIZE];
#pragma HLS ARRAY_PARTITION variable=similarity_matrix cyclic factor=2
	/*
	init_sim_mat_for:for(int i = 0; i < N * (N+M-1); i++){
		similarity_matrix[i] = 0;
	}
	*/

	int similarityDiagonal[N];
#pragma HLS ARRAY_PARTITION variable=similarityDiagonal complete dim=1

	num_diag_for:for(int num_diagonals = 0; num_diagonals < N + M - 1; num_diagonals++){
#pragma HLS inline region recursive
#pragma HLS PIPELINE

		calculate_diagonal(num_diagonals, string1, northwest, north, west, directions_index, compressed_diag, similarityDiagonal, shift_db);
		store_diagonal(directions_index,  direction_matrix_g, compressed_diag, similarity_matrix, similarityDiagonal);
		update_database(string2, shift_db, num_diagonals);
		directions_index ++;
	}

	// Find max val and index in similarity matrix and store in max_index[Row,Column,Value]

	int max_val = 0;
	int max_row = 0;
	int max_col = 0;

	/*
	find_max_sim_for:for(int i = 0; i< N*(N+M-1); i++){
		if(similarity_matrix[i] > max_val) {
			max_val = similarity_matrix[i];
			max_index = i;
		}
	}
	max_index_value[0] = max_index / N;
	max_index_value[1] = max_index % N;
	max_index_value[2] = max_val;
	*/

	find_max_in_simMat_for:for(int i=0; i<(N+M-1); i++){
		if(similarity_matrix[i*2+1] > max_val){
			max_val = similarity_matrix[i*2+1];
			max_col = similarity_matrix[i*2];
			max_row = i;
		}
	}

	max_index_value[0] = max_row;
	max_index_value[1] = max_col;
	max_index_value[2] = max_val;

//	memcpy(direction_matrix_g, direction_matrix, DIRECTION_MATRIX_SIZE * sizeof(short));

	return;
}
}
