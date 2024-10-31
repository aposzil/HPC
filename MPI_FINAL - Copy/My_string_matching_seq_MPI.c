#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include "mpi.h"

#define NUM_THREADS 16

int main (int argc, char *argv[]) {
	
	FILE *pFile;
	long file_size, match_size, pattern_size, total_matches, bsize;
	char * buffer, *local_buffer;
	char * filename, *pattern;
	size_t result;
	int i, j, *match, size, rank;

	//INITIALIZE MPI
	MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

        if (argc != 3) {
		printf ("Usage : %s <file_name> <string>\n", argv[0]);
		return 1;
        }
	filename = argv[1];
	pattern = argv[2];

	pattern_size = strlen(pattern);

	//ONLY RANK 0 reads file
	if (rank==0){

	pFile = fopen ( filename , "rb" );
	if (pFile==NULL) {printf ("File error\n"); return 2;}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	file_size = ftell (pFile);
	rewind (pFile);
	
	printf("file size is %ld\n", file_size);
	
	// allocate memory to contain the file:
	buffer = (char*) malloc (sizeof(char)*file_size);
	if (buffer == NULL) {printf ("Memory error\n"); return 3;}

	// copy the file into the buffer:
	result = fread (buffer,1,file_size,pFile);
	if (result != file_size) {printf ("Reading error\n"); return 4;} 
	
	match_size = file_size - pattern_size + 1;

	match = (int *) malloc (sizeof(int)*match_size);
	if (match == NULL) {printf ("Malloc error\n"); return 5;}

	total_matches = 0;
	for (j = 0; j < match_size; j++){
		match[j]=0;
	}
		bsize = match_size / size;
	}

	double startTime = MPI_Wtime();

	//Barrier to syncronize
	MPI_Barrier(MPI_COMM_WORLD);

	//Broadcast bsize  for loop of brute force
	MPI_Bcast(&bsize,1,MPI_LONG,0,MPI_COMM_WORLD);

	//Scatter part of the buffer
	local_buffer = (char*) malloc (sizeof(char)*bsize);
	MPI_Scatter(buffer,bsize,MPI_CHAR,local_buffer,bsize,MPI_CHAR,0,MPI_COMM_WORLD);
	

	long local_matches = 0;
	double CalcTime = MPI_Wtime();
	// printf("Hallo from %d thread of %d and bsize : %ld and my start is : %ld and my stop = %ld\n",rank,size,bsize,start,stop);
	/* Brute Force string matching */
	for (j = 0; j < bsize; ++j) {
      		for (i = 0; i < pattern_size && pattern[i] == local_buffer[i + j]; ++i);
      		if (i >= pattern_size) {
         		//match[j] = 1;
         		local_matches++;
			}
		}
	CalcTime = MPI_Wtime()-CalcTime;

	//Recution of total matches at process 0 so the total local matches add up
	MPI_Reduce(&local_matches, &total_matches, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

	double stopTime= MPI_Wtime();
	if (rank==0){
		printf("\nTotal matches = %ld\n", total_matches);
		printf("Total times is  : %f\n",stopTime-startTime);
		printf("Total calculation time was : %f\n",CalcTime);
		printf("Total comunication time was : %f\n",stopTime-startTime-CalcTime);
	}
	MPI_Finalize();
	return 0;
}
