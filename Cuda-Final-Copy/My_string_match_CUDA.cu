#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda.h>

__global__
void MyPosition(char *buffer, char *pattern, long match_size, long pattern_size, int *total_matches, int *match)
{
	#include <stdio.h>
	//Deixnei poios ein o arithmos tu thread
  int idx = blockIdx.x*blockDim.x+threadIdx.x;
	int i;

if (idx<match_size){
      for (i = 0; i < pattern_size && pattern[i] == buffer[i + idx]; ++i){
      		if (i >= pattern_size-1) {
         		match[idx] = 1;

						__syncthreads();
         		atomicAdd(total_matches,(long)1);
            }
   }}
}

int main (int argc, char *argv[]) {

	FILE *pFile;
	long file_size, match_size, pattern_size;
	int *total_matches_d;
	char * buffer_h, *buffer_d;
	char * filename, *pattern_h, *pattern_d;
	size_t result;
	int j, *match_h, *match_d;

        if (argc != 4) {
		printf ("Usage : %s <threads_per_block> <file_name> <string>\n", argv[0]);
		return 1;
        }
	filename = argv[2];
	pattern_h = argv[3];
  int threads_per_block = strtol(argv[1], NULL, 10);

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

	pattern_size = strlen(pattern_h);
	match_size = file_size - pattern_size + 2;

	match_h = (int *) malloc (sizeof(int)*match_size);
	if (match_h == NULL) {printf ("Malloc error\n"); return 5;}

	int total_matches_h = 0;
	for (j = 0; j < match_size; j++){
		match_h[j]=0;
	}

	// MyShit

	float total_time, comp_time;
  cudaEvent_t total_start, total_stop, comp_start, comp_stop;
  cudaEventCreate(&total_start);
  cudaEventCreate(&total_stop);
  cudaEventCreate(&comp_start);
  cudaEventCreate(&comp_stop);

	//Set Num of blocks
  int blocks = (match_size+threads_per_block-1)/threads_per_block;

	cudaMalloc((void **)&buffer_d, file_size*sizeof(char));
	cudaMalloc((void **)&pattern_d, pattern_size*sizeof(char));
	cudaMalloc((void **)&total_matches_d,sizeof(long));
	cudaMalloc((void **)&match_d,match_size*sizeof(int));


	// start total timing
  cudaEventRecord(total_start);

	cudaMemcpy(buffer_d, buffer_h, file_size*sizeof(char), cudaMemcpyHostToDevice);
  cudaMemcpy(pattern_d, pattern_h, pattern_size*sizeof(char), cudaMemcpyHostToDevice);
	cudaMemcpy(total_matches_d, &total_matches_h , sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(match_d,match_h,match_size*sizeof(int),cudaMemcpyHostToDevice);

  //Start comp timing
  cudaEventRecord(comp_start);

        /* Brute Force string matching */
	MyPosition<<<blocks,threads_per_block>>>(buffer_d,pattern_d,match_size,pattern_size,total_matches_d,match_d);

	//Stop comp timing and sync
	cudaEventRecord(comp_stop);
  cudaEventSynchronize(comp_stop);
  cudaEventElapsedTime(&comp_time,comp_start,comp_stop);

	//Antigrafes stin mnimi
	cudaMemcpy(&total_matches_h, total_matches_d, sizeof(long), cudaMemcpyDeviceToHost);
	cudaMemcpy(match_h,match_d,match_size*sizeof(int),cudaMemcpyDeviceToHost);

	//Stop total timing and sync
  cudaEventRecord(total_stop);
  cudaEventSynchronize(total_stop);
  cudaEventElapsedTime(&total_time,total_start,total_stop);

	//Typono kai opote vrisko match to proto stoixio kai tin thesi ston buffer
	for (j = 0; j < match_size; j++){
		if(match_h[j]==1){
			printf("[%c,%c,%d]",pattern_h[0],buffer_h[j],j);
		}
	}
        printf("\nTotal matches = %d\n", total_matches_h);

	/*
     * GPU timing
    */
  printf("Total time (ms): %f\n", total_time);
  printf("Kernel time (ms): %f\n", comp_time);
  printf("Data transfer time(ms): %f\n", total_time-comp_time);

	fclose (pFile);
	free (buffer_h);
	free (match_h);
	cudaFree(buffer_d);
  cudaFree(pattern_d);
  cudaFree(total_matches_d);

	return EXIT_SUCCESS;
}