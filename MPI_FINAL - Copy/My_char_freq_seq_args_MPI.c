#include <stdio.h> 
#include <stdlib.h> 
#include "mpi.h"

#define N 128
#define base 0

int main (int argc, char *argv[]) {
	
	FILE *pFile;
	long file_size,bsize;
	char * buffer, *local_buffer;
	char * filename;
	size_t result;
	int i, j, freq[N],*tmp,rank,size;

        if (argc != 2) {
		printf ("Usage : %s <file_name>\n", argv[0]);
		return 1;
        }
	// INITIALIZE MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//ONLY RANK = 0 OPEN FILE 
	if(rank==0){
	filename = argv[1];
	pFile = fopen ( filename , "rb" );
	if (pFile==NULL) {printf ("File error\n"); return 2;}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	file_size = ftell (pFile);
	rewind (pFile);
	//printf("file size is %ld\n", file_size);

	// allocate memory to contain the file:
	buffer = (char*) malloc (sizeof(char)*file_size);
	if (buffer == NULL) {printf ("Memory error\n"); return 3;}

	// copy the file into the buffer only for process 0 :
	
	result = fread (buffer,1,file_size,pFile);
	if (result != file_size) {printf ("Reading error\n"); return 4;} 

	for (j=0; j<N; j++){
		freq[j]=0;
	}
	//Xorizo block size poy tha parei to kathe process 
	bsize=file_size/size;
	}
	
    double startTime = MPI_Wtime();

	//Broadcat the block size from process 0 
	MPI_Bcast(&bsize, 1, MPI_LONG, 0, MPI_COMM_WORLD);

	//Orizo local buffer megethus bsize gia na parei komatia tu buffer
	local_buffer = (char*) malloc (sizeof(char)*bsize);

	//Scatter buffer to local_buffers of bsize fro process 0
	MPI_Scatter(buffer,bsize,MPI_CHAR,local_buffer,bsize,MPI_CHAR,0,MPI_COMM_WORLD);

	tmp = (int *)malloc(N * sizeof(int));

	double CalcTime = MPI_Wtime();

	//Calc itson the freq of chars
	for (i=0; i<bsize; i++){
		tmp[local_buffer[i] - base]++;
	}

	CalcTime = MPI_Wtime() - CalcTime;
	//Reduce each char count from all processses to process 0
	MPI_Reduce(tmp, freq, N, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);	

	double stopTime = MPI_Wtime();
	MPI_Finalize();

	if (rank==0){
	for (j=0; j<N; j++){
		printf("%d = %d\n", j+base, freq[j]);
	}	
	printf("Total time was :%f\n",stopTime-startTime);
	printf("Total Calc time was  : %f\n",CalcTime);
	printf("Total comunication time was  : %f\n",stopTime-startTime-CalcTime);
	fclose (pFile);
	free (buffer);
	}
	
	
	free (local_buffer);

	return 0;
}
