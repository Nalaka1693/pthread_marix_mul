#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define GET_US(X) (X.tv_sec * 1000000 + X.tv_usec)
#define SIZE 10000

void print_mat(char *, int **, int, int);
int **generate_square_matrix(int);
int **serial_matrix_mul(int **, int **, int);
int **serial_trans_matrix_mul(int **, int **, int);
int **transpose(int **, int);

int main() {
    struct timeval start, end;
    struct timezone z;
    long int diff_1, diff_2, diff_3;

    int **result;
    int **mat_1 = generate_square_matrix(SIZE);
    int **mat_2 = generate_square_matrix(SIZE);
    mat_2[2][3] = 4;


    if (gettimeofday(&start, &z)) goto error_exit;
    result = serial_matrix_mul(mat_1, mat_2, SIZE);
    if (gettimeofday(&end, &z)) goto error_exit;
    diff_1 = GET_US(end) - GET_US(start);
    printf("mul time = %ld\n", diff_1);

    if (gettimeofday(&start, &z)) goto error_exit;
    int **mat_2_t = transpose(mat_2, SIZE);
    if (gettimeofday(&end, &z)) goto error_exit;
    diff_2 = GET_US(end) - GET_US(start);
    printf("t time = %ld\n", diff_2);

    if (gettimeofday(&start, &z)) goto error_exit;
    result = serial_trans_matrix_mul(mat_1, mat_2_t, SIZE);
    if (gettimeofday(&end, &z)) goto error_exit;
    diff_3 = GET_US(end) - GET_US(start);
    printf("t mul time = %ld\n", diff_3);

    printf("performance gain = %ld - (%ld + %ld) = %ld\n", diff_1, diff_3, diff_2, diff_1 - (diff_2 + diff_3));

    return 0;

    error_exit:
    printf("cannot read time.\n");
    return -1;
}

int **parallel_trans_matrix_mul(int **a, int **b, int size) {
    return NULL;
}

int **parallel_matrix_mul(int **a, int **b, int size, int thread_count) {

}

int **serial_trans_matrix_mul(int **a, int **b, int size) {
    int i, j, k, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;

    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        if (!row) goto malloc_err;
        matrix[i] = row;

        for (j = 0; j < size; j++) {
            int ele_total = 0;
            for (k = 0; k < size; k++) {
                ele_total += a[i][k] * b[j][k];
            }

            matrix[i][j] = ele_total;
        }
    }

    return matrix;

    malloc_err:
    printf("cannot allocate memory.\n");
    return NULL;
}

int **transpose(int **a, int size) {
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

int **serial_matrix_mul(int **a, int **b, int size) {
    int i, j, k, **matrix = (int **) malloc(sizeof(int *) * size);
    if (!matrix) goto malloc_err;

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