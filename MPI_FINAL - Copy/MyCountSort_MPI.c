#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define UPPER 1000
#define LOWER 0

// Comparison function for qsort
int cmpfunc(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

int main(int argc, char *argv[]) {
    int rank, size, *x, *y, *tmp, *y2;
    int i, j, my_num, my_place;

    if (argc != 2) {
        printf("Usage : %s <array_size>\n", argv[0]);
        return 1;
    }

    int n = strtol(argv[1], NULL, 10);
    x = (int *)malloc(n * sizeof(int));
    y = (int *)malloc(n * sizeof(int));
    // a = (int *)malloc(n * sizeof(int));

    for (i = 0; i < n; i++) {
        x[i] = n - i;
        // x[i] = (rand() % (UPPER - LOWER + 1)) + LOWER;
    }

    // INITIALIZE MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    double startTime = MPI_Wtime();
    

    // SET BLOCK SIZE
    int bsize = (int)(n / size);

    tmp = (int *)malloc(bsize * sizeof(int));
    //Scater bsize size of blocks of x to the prosseses stored as tmo
    MPI_Scatter(x, bsize, MPI_INT, tmp, bsize, MPI_INT, 0, MPI_COMM_WORLD);

    double CalcTime = MPI_Wtime();

    y2 = (int *)malloc(bsize * sizeof(int));
    for (j = 0; j < bsize; j++) {
        my_num = tmp[j];
        my_place = 0;
        for (i = 0; i < bsize; i++)
            if ((my_num > tmp[i]) || ((my_num == tmp[i]) && (j < i)))
                my_place++;
        y2[my_place] = my_num;
    }
    CalcTime = MPI_Wtime() - CalcTime;
    //Gather Bsize of sorted arrays y2  to store them at y so we sort the y  
    MPI_Gather(y2, bsize, MPI_INT, y, bsize, MPI_INT, 0, MPI_COMM_WORLD);

    // Sort the array y using qsort
    double totalTime = MPI_Wtime() - startTime;
    if (rank == 0) {
        qsort(y, n, sizeof(int), cmpfunc);
    }

    MPI_Finalize();

    if (rank == 0) {
        printf("HALLO N IS : %d\n", n);
        for (i = 0; i < n; i++) {
            printf("%d\n", y[i]);
        }
        printf("Total Time was : %f\n", totalTime);
        printf("Total Calc time was : %f\n",CalcTime);
        printf("Total comunication time was  : %f\n",totalTime-CalcTime);

    }

    // // Free allocated memory
    // free(x);
    // free(y);
    // free(a);
    // free(tmp);
    // free(y2);

    return 0;
}

