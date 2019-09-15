#include "libspir_types.h"
#include "hls_stream.h"
#include "xcl_top_defines.h"
#include "ap_axi_sdata.h"
#define EXPORT_PIPE_SYMBOLS 1
#include "cpu_pipes.h"
#undef EXPORT_PIPE_SYMBOLS
#include "xcl_half.h"
#include <cstddef>
#include <vector>
#include <pthread.h>

extern "C" {

void compute_matrices(size_t string1, size_t string2, size_t max_index, size_t similarity_matrix, size_t direction_matrix);

static pthread_mutex_t __xlnx_cl_compute_matrices_mutex = PTHREAD_MUTEX_INITIALIZER;
void __stub____xlnx_cl_compute_matrices(char **argv) {
  void **args = (void **)argv;
  size_t string1 = *((size_t*)args[0+1]);
  size_t string2 = *((size_t*)args[1+1]);
  size_t max_index = *((size_t*)args[2+1]);
  size_t similarity_matrix = *((size_t*)args[3+1]);
  size_t direction_matrix = *((size_t*)args[4+1]);
  pthread_mutex_lock(&__xlnx_cl_compute_matrices_mutex);
  compute_matrices(string1, string2, max_index, similarity_matrix, direction_matrix);
  pthread_mutex_unlock(&__xlnx_cl_compute_matrices_mutex);
}
}
