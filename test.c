#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define GET_US(X) (X.tv_sec * 1000000 + X.tv_usec)
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

void print_mat(char *, int **, int, int);
void analyze(void);
int **generate_square_matrix(int);
int **serial_matrix_mul(int **, int **, int, int);
int **parallel_matrix_mul(int **, int **, int, int, int);
int **serial_transpose(int **, int);
int **parallel_transpose(int **, int, int);

void *thread_routine_mul_ntrans(void *arg) {
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
}

int main() {
    analyze();

    return 0;
}

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
        } else if (thread_count * 2 > size) {
            pthread_t threads[thread_count - 1];
            for (i = 0; i < thread_count - 1; i++) {

            }
        } else {

        }
    } else {
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
                thread_data->mat_2 = b;
                thread_data->result = matrix;
                thread_data->size = size;
                assert(!pthread_create(&threads[i], NULL, thread_routine_mul_ntrans, (void *) thread_data));
            }

            for (i = 0; i < thread_count; i++) {
                pthread_join(threads[i], NULL);
            }
        } else if (thread_count * 2 > size) {
            pthread_t threads[thread_count - 1];
            for (i = 0; i < thread_count - 1; i++) {

            }
        } else {

        }
    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

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
        for (i = 0; i < thread_count - 1; i++) {

        }
    } else {

    }


    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

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

void print_mat(char *msg, int **mat, int rows, int cols) {
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

void analyze(void) {
    FILE *f = fopen("data.txt", "w");

    struct timeval start, end;
    struct timezone z;
    long int diff_1;

    fprintf(f, "size\ts_m,t_n\t\ts_m,t_s\t\ts_m,t_p\t\tp_m,t_n\t\tp_m,t_s\t\tp_m,t_p\n");

    int i, **result;
    for (i = 1000; i < 10000; i += 1000) {
        int **mat_1 = generate_square_matrix(i);
        int **mat_2 = generate_square_matrix(i);

        fprintf(f, "%d\t", i);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = serial_matrix_mul(mat_1, mat_2, i, 0);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        fprintf(f, "%ld\t", diff_1);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = serial_matrix_mul(mat_1, mat_2, i, 1);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        fprintf(f, "%ld\t", diff_1);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = serial_matrix_mul(mat_1, mat_2, i, 2);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        fprintf(f, "%ld\t", diff_1);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = parallel_matrix_mul(mat_1, mat_2, i, THREADS, 0);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        fprintf(f, "%ld\t", diff_1);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = parallel_matrix_mul(mat_1, mat_2, i, THREADS, 1);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        fprintf(f, "%ld\t", diff_1);

        if (gettimeofday(&start, &z)) goto error_exit;
        result = parallel_matrix_mul(mat_1, mat_2, i, THREADS, 2);
        if (gettimeofday(&end, &z)) goto error_exit;
        diff_1 = GET_US(end) - GET_US(start);
        fprintf(f, "%ld\n", diff_1);
    }

    return;

    error_exit:
    printf("cannot read time.\n");
    return;
}