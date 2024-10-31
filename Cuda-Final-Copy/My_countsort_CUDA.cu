#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

#define UPPER 1000
#define LOWER 0

__global__
void MyPosition(int *x, int *y, int n)
{
  //Deixnei poios ein o arithmos tu thread
  int idx = blockIdx.x*blockDim.x+threadIdx.x;
  //printf("HALLO FROM THREAD : %d",idx);

if (idx<n){
	int my_num=x[idx];
    int my_place = 0;
     for (int i=0; i<n; i++)
		if ((my_num > x[i]) || ((my_num == x[i]) && (idx < i)))
			my_place++;
      __syncthreads();
     y[my_place] = my_num;
   }
}

int main(int argc, char *argv[])
{
   int *x_h, *y_h, *x_d, *y_d;
   int i;

   cudaEvent_t total_start, total_stop, comp_start, comp_stop;
   float total_time, comp_time;
   cudaEventCreate(&total_start);
   cudaEventCreate(&total_stop);
   cudaEventCreate(&comp_start);
   cudaEventCreate(&comp_stop);

   if (argc != 3) {
		printf ("Usage : %s <array_size> <threads_per_block> \n", argv[0]);
		return 1;
   }

   const int n = strtol(argv[1], NULL, 10);
   const int threads_per_block = strtol(argv[2],NULL,10);
   //Memory on host
   x_h = ( int * ) malloc ( n * sizeof ( int ) );
   y_h = ( int * ) malloc ( n * sizeof ( int ) );

   if (x_h == NULL || y_h == NULL) {
        fprintf(stderr, "Failed to allocate host memory.\n");
        return 1;
    }

    // Allocate memory on device
    if (cudaMalloc((void **)&x_d, n * sizeof(int)) != cudaSuccess) {
        fprintf(stderr, "Failed to allocate device memory X.\n");
        return 1;
    }else if(cudaMalloc((void **)&y_d, n * sizeof(int)) != cudaSuccess){
      fprintf(stderr, "Failed to allocate device memory Y.\n");
        return 1;
    }

   //Memory on device
   cudaMalloc((void **) &x_d,n * sizeof (int));
   cudaMalloc((void **) &y_d,n * sizeof (int));


   for (i=0; i<n; i++)
		x_h[i] = n - i;
		//x[i] = (rand() % (UPPER - LOWER + 1)) + LOWER;

    //Set Num of blocks
    int blocks = (n+threads_per_block-1)/threads_per_block;

    // start total timing
    cudaEventRecord(total_start);

    //Memory copy
    cudaMemcpy( x_d, x_h, n*sizeof(int), cudaMemcpyHostToDevice );
    cudaMemcpy( y_d, y_h, n*sizeof(int), cudaMemcpyHostToDevice );

  //Start comp timing
    cudaEventRecord(comp_start);

    printf("REACHED HERE \n");

    MyPosition<<<blocks,threads_per_block>>>(x_d, y_d, n);

    //Stop comp timing and sync
    cudaEventRecord(comp_stop);
    cudaEventSynchronize(comp_stop);
    cudaEventElapsedTime(&comp_time,comp_start,comp_stop);


    cudaMemcpy(y_h ,y_d, n*sizeof(int),cudaMemcpyDeviceToHost);

    //Stop total timing and sync
    cudaEventRecord(total_stop);
    cudaEventSynchronize(total_stop);
    cudaEventElapsedTime(&total_time,total_start,total_stop);

    for (i=n-15; i<n; i++)
		printf("%d\n", y_h[i]);

    /*
     * GPU timing
    */
    printf("Total time (ms): %f\n", total_time);
    printf("Kernel time (ms): %f\n", comp_time);
    printf("Data transfer time(ms): %f\n", total_time-comp_time);

    cudaFree(x_d);
    cudaFree(y_d);

   return EXIT_SUCCESS;
}