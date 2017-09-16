/* matrix multiplication performance tested on six scenarios
 *
 * s_m,n_t -> serial multiplication and both untransposed matrices
 * s_m,s_t -> serial multiplication and one serially transposed matrix
 * s_m,p_t -> serial multiplication and one parallelly transposed matrix
 * p_m,n_t -> parallel multiplication and both untransposed matrices
 * p_m,s_t -> parallel multiplication and one serially transposed matrix
 * p_m,p_t -> parallel multiplication and one parallelly transposed matrix
 *
 * all times are in seconds with micro second precision
 *
 * corei3
 * size    s_m,n_t         s_m,s_t         s_m,p_t         p_m,n_t         p_m,s_t         p_m,p_t
 * 1000    11.655115       4.375538 	   4.375467        5.281501 	   2.281360 	   2.282026
 * 2000    113.718077      35.065581       34.737341       50.589790       18.350671       18.330118
 * 3000    486.600354      116.169977      116.321871      218.822065      61.373005       61.282945
 * 4000    1316.753109     275.491810      274.913886      621.828521
 * 
 * dual core
 * size    s_m,t_n         s_m,t_s         s_m,t_p         p_m,t_n         p_m,t_s         p_m,t_p
 * 1000    11.667001       5.063219        5.151984        6.653818        2.909295        3.139972
 * 2000    105.010285      49.862603       49.013855       63.431698       31.667863       31.520016
 * 3000    405.391298      161.887154      161.420638      140.135249      107.059176      106.877331
 
 * xeon
 * size    s_m,n_t         s_m,s_t         s_m,p_t         p_m,n_t         p_m,s_t         p_m,p_t
 * 1000    6.765998        3.300273        3.301610        1.716495        0.852855        1.215227
 * 2000    63.787786       26.363870       26.351717       7.732306        6.753414        9.240850
 * 3000    181.629340      91.507772       91.498154       47.050190       23.631547       23.466153
 *
 * serial_matrix_mul: function multiply 2 matrices serially element by
 *  element. time complexity O(n^3)
 *
 * serial_transpose: function transposing one matrix serially. time complexity O(n^2)
 *
 * parallel_matrix_mul: function multiply 2 matrices parallely. decompsite
 *  data by rows of the result matrix. sets of rows are given to different
 *  threads. time complexity O(n^3)
 *
 * parallel_transpose: function transposing one matrix parallelly. time complexity O(n^2).
 *  decompsite data by rows of the result matrix.
 *
 * serial multiply cost more time but transposing second matrix will give large performance
 * improvement because of the efficient cache access. parallel multipication and prallel
 * transpose will cost more time for small matrix sizes because of the thread creation cost
 * (only at size 1000). best performance given when using the both parallel multiplication
 * and transpose functions.
 *
 */


#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define GET_US(X) (X.tv_sec * 1000000 + X.tv_usec)
// only work for size % threads == 0
#define SIZE 1000
#define THREADS 4

typedef struct {
    int start;
    int end;
    int **mat_1;
    int **mat_2;
    int **result;
    int size;
} thread_data_t;

void print_matrix(char *, int **, int, int);
void matrix_mem_free(int **, int);
void analyze(void);
int **generate_square_matrix(int);
int **serial_matrix_mul(int **, int **, int, int);
int **parallel_matrix_mul(int **, int **, int, int, int);
int **serial_transpose(int **, int);
int **parallel_transpose(int **, int, int);

/* thread routine for both non transeposed matrices
 */
void *thread_routine_mul_untrans(void *arg) {
    int start = ((thread_data_t *) arg)->start;
    int end = ((thread_data_t *) arg)->end;
    int **mat_1 = ((thread_data_t *) arg)->mat_1;
    int **mat_2 = ((thread_data_t *) arg)->mat_2;
    int **result = ((thread_data_t *) arg)->result;
    int size = ((thread_data_t *) arg)->size;

    int i, j, k;
    for (i = start; i < end; i++) {
        for (j = 0; j < size; j++) {
            int ele_total = 0;
            for (k = 0; k < size; k++) {
                ele_total += mat_1[i][k] * mat_2[k][j];
            }

            result[i][j] = ele_total;
        }
    }

    pthread_exit(NULL);
}

/* thread routine for both one transeposed matrices
 */
void *thread_routine_mul_trans(void *arg) {
    int start = ((thread_data_t *) arg)->start;
    int end = ((thread_data_t *) arg)->end;
    int **mat_1 = ((thread_data_t *) arg)->mat_1;
    int **mat_2 = ((thread_data_t *) arg)->mat_2;
    int **result = ((thread_data_t *) arg)->result;
    int size = ((thread_data_t *) arg)->size;

    int i, j, k;
    for (i = start; i < end; i++) {
        for (j = 0; j < size; j++) {
            int ele_total = 0;
            for (k = 0; k < size; k++) {
                ele_total += mat_1[i][k] * mat_2[j][k];
            }

            result[i][j] = ele_total;
        }
    }

    pthread_exit(NULL);
}

/* thread routine for transposing a matrix
 */
void *thread_routine_transpose(void *arg) {
    int start = ((thread_data_t *) arg)->start;
    int end = ((thread_data_t *) arg)->end;
    int **mat_1 = ((thread_data_t *) arg)->mat_1;
    int **result = ((thread_data_t *) arg)->result;
    int size = ((thread_data_t *) arg)->size;

    int i, j;
    for (i = start; i < end; i++) {
        for (j = 0; j < size; j++) {
            result[i][j] = mat_1[j][i];
        }
    }

    pthread_exit(NULL);
}

int main() {
    analyze();

    return 0;
}

/* parallel matrix multiplication
 * @ a: first square matrix
 * @ b: second square matrix
 * @ size: size of the square matrix
 * @ thread_count: number of threads
 * @ transposed: 0 -> both matrices not transposed
 *               1 -> one matrix transposed serially
 *               2 -> one matrix transposed parallely
 */
int **parallel_matrix_mul(int **a, int **b, int size, int thread_count, int transposed) {
    int i, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;
    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        if (!row) goto malloc_err;
        matrix[i] = row;
    }

    if (transposed) {
        int **b_t;
        if (transposed == 1) {
            b_t = serial_transpose(b, size);
        } else if (transposed == 2) {
            b_t = parallel_transpose(b, size, thread_count);
        }

        if (size < thread_count) {
            pthread_t threads[size];
            for (i = 0; i < size; i++) {

            }
        } else if (size % thread_count == 0) {
            int frac = size / thread_count;
            pthread_t threads[thread_count];

            for (i = 0; i < thread_count; i++) {
                thread_data_t *thread_data = (thread_data_t *) malloc(sizeof(thread_data_t));
                int iintof = i * frac;
                thread_data->start = iintof;
                thread_data->end = iintof + frac;
                thread_data->mat_1 = a;
                thread_data->mat_2 = b_t;
                thread_data->result = matrix;
                thread_data->size = size;
                assert(!pthread_create(&threads[i], NULL, thread_routine_mul_trans, (void *) thread_data));
            }

            for (i = 0; i < thread_count; i++) {
                pthread_join(threads[i], NULL);
            }
            matrix_mem_free(b_t, size);
        } else if (thread_count * 2 > size) {
            pthread_t threads[thread_count - 1];
            for (i = 0; i < thread_count - 1; i++) {

            }
        } else {

        }
    } else {
        if (size < thread_count) {
            pthread_t threads[size];
            // TODO  when size is less than the thread count
        } else if (size % thread_count == 0) {
            int frac = size / thread_count;
            pthread_t threads[thread_count];
            for (i = 0; i < thread_count; i++) {
                thread_data_t *thread_data = (thread_data_t *) malloc(sizeof(thread_data_t));
                int iintof = i * frac;
                thread_data->start = iintof;
                thread_data->end = iintof + frac;
                thread_data->mat_1 = a;
                thread_data->mat_2 = b;
                thread_data->result = matrix;
                thread_data->size = size;
                assert(!pthread_create(&threads[i], NULL, thread_routine_mul_untrans, (void *) thread_data));
            }

            for (i = 0; i < thread_count; i++) {
                pthread_join(threads[i], NULL);
            }
        } else if (thread_count * 2 > size) {
            pthread_t threads[thread_count - 1];
            // TODO
        } else {

        }
    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

/* parallel matrix transpose
 * @ a: square matrix
 * @ size: size of the square matrix
 * @ thread_count: number of threads
 */
int **parallel_transpose(int **a, int size, int thread_count) {
    int i, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;
    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        if (!row) goto malloc_err;
        matrix[i] = row;
    }

    if (size < thread_count) {
        pthread_t threads[size];
        // TODO when size id less than the thread count
    } else if (size % thread_count == 0) {
        int frac = size / thread_count;
        pthread_t threads[thread_count];

        for (i = 0; i < thread_count; i++) {
            thread_data_t *thread_data = (thread_data_t *) malloc(sizeof(thread_data_t));
            int iintof = i * frac;
            thread_data->start = iintof;
            thread_data->end = iintof + frac;
            thread_data->mat_1 = a;
            thread_data->mat_2 = NULL;
            thread_data->result = matrix;
            thread_data->size = size;
            assert(!pthread_create(&threads[i], NULL, thread_routine_transpose, (void *) thread_data));
        }

        for (i = 0; i < thread_count; i++) {
            pthread_join(threads[i], NULL);
        }
    } else if (thread_count * 2 > size) {
        pthread_t threads[thread_count - 1];
        // TODO
    } else {

    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

/* serial matrix multiplication
 * @ a: first square matrix
 * @ b: second square matrix
 * @ size: size of the square matrix
 * @ thread_count: number of threads
 * @ transposed: 0 -> both matrices not transposed
 *               1 -> one matrix transposed serially
 *               2 -> one matrix transposed parallely
 */
int **serial_matrix_mul(int **a, int **b, int size, int transposed) {
    int i, j, k, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;

    if (transposed) {
        int **b_t;
        if (transposed == 1) {
            b_t = serial_transpose(b, size);
        } else if (transposed == 2) {
            b_t = parallel_transpose(b, size, THREADS);
        }

        for (i = 0; i < size; i++) {
            int *row = (int *) malloc(sizeof(int) * size);
            if (!row) goto malloc_err;
            matrix[i] = row;

            for (j = 0; j < size; j++) {
                int ele_total = 0;
                for (k = 0; k < size; k++) {
                    ele_total += a[i][k] * b_t[j][k];
                }

                matrix[i][j] = ele_total;
            }
        }
        matrix_mem_free(b_t, size);
    } else {
        for (i = 0; i < size; i++) {
            int *row = (int *) malloc(sizeof(int) * size);
            if (!row) goto malloc_err;
            matrix[i] = row;

            for (j = 0; j < size; j++) {
                int ele_total = 0;
                for (k = 0; k < size; k++) {
                    ele_total += a[i][k] * b[k][j];
                }

                matrix[i][j] = ele_total;
            }
        }
    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

/* serial matrix transpose
 * @ a: square matrix
 * @ size: size of the square matrix
 */
int **serial_transpose(int **a, int size) {
    int i, j, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;

    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        if (!row) goto malloc_err;
        matrix[i] = row;

        for (j = 0; j < size; j++) {
            matrix[i][j] = a[j][i];
        }
    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

/* generate square matrix
 * @ size: size of the square matrix
 */
int **generate_square_matrix(int size) {
    int i, j, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;

    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        if (!row) goto malloc_err;

        matrix[i] = row;
        for (j = 0; j < size; j++) {
            row[j] = 1;
        }
    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

/* print matrix
 * @ msg: messege to be printed
 * @ mat: reference to the matrix need to be printed
 * @ rows: number of rows of square matrix
 * @ cols: number of columns of square matrix
 */
void print_matrix(char *msg, int **mat, int rows, int cols) {
    int i, j;
    if (msg) {
        printf("%s ", msg);
    }
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%d ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* matrix memory free
 * @ mat: reference to the matrix need to be freed
 * @ size: size of the square matrix
 */
void matrix_mem_free(int **mat, int size) {
    int i;
    for (i = 0; i < size; i++) {
        free(mat[i]);
    }
    free(mat);
}

/* some analyzing code for performance comparing
 */
void analyze(void) {
    struct timeval start, end;
    struct timezone z;
    double diff_1;

    printf("size\ts_m,n_t\t\ts_m,s_t\t\ts_m,p_t\t\tp_m,n_t\t\tp_m,s_t\t\tp_m,p_t\n");

    int i, **result;
    // only work for i % threads == 0
    for (i = 1000; i < 10000; i += 1000) {
        int **mat_1 = generate_square_matrix(i);
        int **mat_2 = generate_square_matrix(i);

        printf("%d\t", i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = serial_matrix_mul(mat_1, mat_2, i, 0);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        printf("%f\t", diff_1 / 1000000.0);
        matrix_mem_free(result, i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = serial_matrix_mul(mat_1, mat_2, i, 1);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        printf("%f\t", diff_1 / 1000000.0);
        matrix_mem_free(result, i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = serial_matrix_mul(mat_1, mat_2, i, 2);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        printf("%f\t", diff_1 / 1000000.0);
        matrix_mem_free(result, i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = parallel_matrix_mul(mat_1, mat_2, i, THREADS, 0);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        printf("%f\t", diff_1 / 1000000.0);
        matrix_mem_free(result, i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = parallel_matrix_mul(mat_1, mat_2, i, THREADS, 1);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        printf("%f\t", diff_1 / 1000000.0);
        matrix_mem_free(result, i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = parallel_matrix_mul(mat_1, mat_2, i, THREADS, 2);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        printf("%f\t", diff_1 / 1000000.0);
        matrix_mem_free(result, i);

        matrix_mem_free(mat_1, i);
        matrix_mem_free(mat_2, i);
    }

    return;

    error_exit:
    printf("cannot read time.\n");
    return;
}
