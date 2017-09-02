#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define GET_US(X) (X.tv_sec * 1000000 + X.tv_usec)
#define SIZE 100

void print_mat(char *, int **, int, int);
int **generate_square_matrix(int);
int **serial_matrix_mul(int **, int **, int);
int **transpose(int **, int);

int main() {
    struct timeval start, end;
    struct timezone z;
    long int diff;

    int **result;
    int **mat_1 = generate_square_matrix(SIZE);
    int **mat_2 = generate_square_matrix(SIZE);

    if (gettimeofday(&start, &z)) goto error_exit;
    result = serial_matrix_mul(mat_1, mat_2, SIZE);
    if (gettimeofday(&end, &z)) goto error_exit;
    diff = GET_US(end) - GET_US(start);
    printf("Time = %ld\n", diff);

    print_mat(NULL, result, SIZE, SIZE);


    return 0;

    error_exit:
    printf("Cannot read time.\n");
    return -1;
}

int **transpose(int **a, int size) {
    int i, j, **matrix = (int **) malloc(sizeof(int *) * size);
    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        for (j = 0; j < size; j++) {
            matrix[i][j] = a[j][i];
        }
    }

    return matrix;
}

int **serial_matrix_mul(int **a, int **b, int size) {
    int i, j, k,**matrix = (int **) malloc(sizeof(int *) * size);
    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        matrix[i] = row;
        for (j = 0; j < size; j++) {
            int ele_total = 0;
            for (k = 0; k < size; k++) {
                ele_total += a[i][k] * b[k][j];
            }

            matrix[i][j] = ele_total;
        }
    }

    return  matrix;
}

int **generate_square_matrix(int size) {
    int i, j, **matrix = (int **) malloc(sizeof(int *) * size);
    for (i = 0; i < size; i++) {
        int *row = (int *) malloc(sizeof(int) * size);
        matrix[i] = row;
        for (j = 0; j < size; j++) {
            row[j] = 1;
        }
    }

    return matrix;
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