#include "stuff.h"


int main(int argc, char *argv[]) 
{
    int absRank, numnodes;
    MPI_Comm comm;
    
    MPI_Init(&argc, &argv);
  
    MPI_Comm_rank(MPI_COMM_WORLD, &absRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numnodes); 
    
    
    
    int numberOfGroups = atoi(argv[1]);
    int digitsToCount = atoi(argv[2]); 

    int error;
  
    int color = MPI_Rand_Split(numberOfGroups, absRank, numnodes, &comm); 
    
    char outputFilename[20];
    sprintf(outputFilename, "output%d.txt", color);

    int myrank;
    MPI_Comm_rank(comm, &myrank);
    
    int groupSize;
    MPI_Comm_size(comm, &groupSize);
    
    MPI_File outputFile;  

    int slaveProcessDigitsCount = digitsToCount / groupSize;
    int masterProcessDigitsCount = digitsToCount - ((groupSize - 1) * slaveProcessDigitsCount);

    int length = (myrank == 0) ? masterProcessDigitsCount : slaveProcessDigitsCount;
    int offset = (myrank == 0) ? 0 : masterProcessDigitsCount + (myrank - 1) * slaveProcessDigitsCount;

    double startTime;
    if (myrank == 0)
    {
        startTime = MPI_Wtime();
    }

    char *digits = (char *)malloc(sizeof(char) * length);

    for (int count = 0; count < length; count++)
    {
        digits[count] = calculatePiDigit(count + offset);
    }

    MPI_File_open(comm, outputFilename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &outputFile);


    error = MPI_File_set_view(outputFile, offset, MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
    MPIerror(error, myrank);

    error = MPI_File_write_all(outputFile, digits, length, MPI_CHAR, MPI_STATUS_IGNORE);
    MPIerror(error, myrank);

    error = MPI_File_close(&outputFile);
    MPIerror(error, myrank);

    free(digits);

    if (myrank == 0)
    {
        printf("Group #%d, processes count %d, time is %f\n\n", color, groupSize, MPI_Wtime() - startTime);
    }

    MPI_Finalize();
    return 0;
}



