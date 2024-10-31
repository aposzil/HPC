#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"

#define NUM_THREADS 16

void main ( int argc, char *argv[] )  {

int   i, j, N, rank, size, bsize;
float *x, *b, **a, sum,local_sum, *local_a, *local_x;
char any;

	if (argc != 2) {
		printf ("Usage : %s <matrix size>\n", argv[0]);
                exit(1);
	}

	//INITIALIZE
	MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	if(rank==0){

	N = strtol(argv[1], NULL, 10);

	/* Allocate space for matrices */
	a = (float **) malloc ( N * sizeof ( float *) );
	for ( i = 0; i < N; i++) 
		a[i] = ( float * ) malloc ( (i+1) * sizeof ( float ) );
	b = ( float * ) malloc ( N * sizeof ( float ) );
	x = ( float * ) malloc ( N * sizeof ( float ) );

	/* Create floats between 0 and 1. Diagonal elents between 2 and 3. */
	srand ( time ( NULL));
	for (i = 0; i < N; i++) {
		x[i] = 0.0;
		b[i] = (float)rand()/(RAND_MAX*2.0-1.0);
		a[i][i] = 2.0+(float)rand()/(RAND_MAX*2.0-1.0);
		for (j = 0; j < i; j++) 
			a[i][j] = (float)rand()/(RAND_MAX*2.0-1.0);;
	} }

	double startTime = MPI_Wtime();

	//BCast the N from process 0
	MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
	double totalCalcTime =0;

	//delcare local vars of each process
	bsize = N/size;
	local_a = (float *) malloc ( bsize * sizeof ( float ) );
	local_x = ( float * ) malloc ( bsize * sizeof ( float ) );

        /* Calulation */
	for (i = 0; i < N; i++) {
		local_sum = 0.0;
		if (rank==0){
			sum=0.0;
		}

		//Scatter x and a for calculations
		MPI_Scatter(x, bsize, MPI_FLOAT, local_x, bsize, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Scatter(&a[i],bsize,MPI_FLOAT,local_a,bsize,MPI_FLOAT,0,MPI_COMM_WORLD);
		double CalcTime = MPI_Wtime();
		
		for (j = 0; j < bsize; j++) {
			if(j<i){
				local_sum = local_sum + (local_x[j] * local_a[j]);
			}
			//printf ("%d %d %f %f %f \t \n", i, j, x[j], a[i][j], sum);
		}

		totalCalcTime=totalCalcTime+MPI_Wtime()-CalcTime;
		
		//Barrier to be sure to sicronie before reduce
		MPI_Barrier(MPI_COMM_WORLD);

		//Recuse the total sum from local
		MPI_Reduce(&local_sum, &sum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
		if(rank==0){
			x[i] = (b[i] - sum) / a[i][i];
		}
		//printf ("%d %f %f %f %f \n", i, b[i], sum, a[i][i], x[i]);
	}
	double stopTime = MPI_Wtime();
    /* Print result 
	for (i = 0; i < N; i++) {
		for (j = 0; j <= i; j++)
			printf ("%f \t", a[i][j]);	
		printf ("%f \t%f\n", x[i], b[i]);
	}*/

   /* Check result */
   if(rank==0){
	for (i = 0; i < N; i++) {
		sum = 0.0;
		for (j = 0; j <= i; j++) 
			sum = sum + (x[j]*a[i][j]);	
		if (fabsf(sum - b[i]) > 0.00001) {
			printf("%f != %f\n", sum, b[i]);
			printf("Validation Failed...\n");
		}
	}
	printf("Total time was : %f\n",stopTime-startTime);
	printf("Total Calcuilation time was : %f\n",totalCalcTime);
	printf("Total Communication time was  : %f\n",stopTime-startTime-totalCalcTime);
	}
	MPI_Finalize();
}
