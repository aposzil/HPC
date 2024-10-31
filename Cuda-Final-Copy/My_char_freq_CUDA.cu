#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>

#define N 128
#define base 0

__global__
void MyPosition(char *buffer, int *freq, int n)
{
  //Deixnei poios ein o arithmos tu thread
  int idx = blockIdx.x*blockDim.x+threadIdx.x;
  //printf("HALLO FROM THREAD : %d",idx);

if (idx<n){
      atomicAdd(&freq[buffer[idx]-base], 1);
   }
}

int main (int argc, char *argv[]) {

	FILE *pFile;
	long file_size;
	char * buffer_h,*buffer_d;
	char * filename;
	size_t result;
	int  j, freq_h[N],*freq_d;

        if (argc != 3) {
		printf ("Usage : %s <file_name> <threads_per_bock>\n", argv[0]);
		return 1;
        }
	filename = argv[1];
  int threads_per_block = strtol(argv[2], NULL, 10);
	pFile = fopen ( filename , "rb" );
	if (pFile==NULL) {printf ("File error\n"); return 2;}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	file_size = ftell (pFile);
	rewind (pFile);
	printf("file size is %ld\n", file_size);

	// allocate memory to contain the file:
	buffer_h = (char*) malloc (sizeof(char)*file_size);
	if (buffer_h == NULL) {printf ("Memory error\n"); return 3;}

	// copy the file into the buffer:
	result = fread (buffer_h,1,file_size,pFile);
	if (result != file_size) {printf ("Reading error\n"); return 4;}

	for (j=0; j<N; j++){
		freq_h[j]=0;
	}

  //MyShit---------------------

  float total_time, comp_time;
    cudaEvent_t total_start, total_stop, comp_start, comp_stop;
    cudaEventCreate(&total_start);
    cudaEventCreate(&total_stop);
    cudaEventCreate(&comp_start);
    cudaEventCreate(&comp_stop);

  //Set Num of blocks
    int blocks = (file_size+threads_per_block-1)/threads_per_block;

    cudaMalloc((void **)&buffer_d, file_size*sizeof(char));
    cudaMalloc((void **)&freq_d, N*sizeof(int));

    // start total timing
    cudaEventRecord(total_start);

    cudaMemcpy(buffer_d, buffer_h, file_size*sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(freq_d, freq_h, N*sizeof(int), cudaMemcpyHostToDevice);

    //Start comp timing
    cudaEventRecord(comp_start);

    MyPosition<<<blocks,threads_per_block>>>(buffer_d,freq_d,file_size);

    //Stop comp timing and sync
    cudaEventRecord(comp_stop);
    cudaEventSynchronize(comp_stop);
    cudaEventElapsedTime(&comp_time,comp_start,comp_stop);

    cudaMemcpy(freq_h, freq_d, N*sizeof(int), cudaMemcpyDeviceToHost);

    //Stop total timing and sync
    cudaEventRecord(total_stop);
    cudaEventSynchronize(total_stop);
    cudaEventElapsedTime(&total_time,total_start,total_stop);

  int total_elements=0;

	for (j=0; j<N; j++){
		printf("%d = %d\n", j+base, freq_h[j]);
    total_elements+=freq_h[j];
	}
  printf("Total elements are %d\n",total_elements);
   /*
     * GPU timing
    */
    printf("Total time (ms): %f\n", total_time);
    printf("Kernel time (ms): %f\n", comp_time);
    printf("Data transfer time(ms): %f\n", total_time-comp_time);


    cudaFree(buffer_d);
    cudaFree(freq_d);

	fclose (pFile);
	free (buffer_h);

	return 0;
}