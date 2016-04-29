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
    
    int color = MPI_Rand_Split(numberOfGroups, absRank, numnodes, &comm); 
    
    int myrank;
    MPI_Comm_rank(comm, &myrank);    
    
    int groupSize;
    MPI_Comm_size(comm, &groupSize);

    
    int slaveProcessDigitsCount = digitsToCount / groupSize;
    int masterProcessDigitsCount = digitsToCount - ((groupSize - 1) * slaveProcessDigitsCount);
    
    double startTime;  
    if (myrank == 0) 
    {
        printf("master %d\n slave %d\n", masterProcessDigitsCount, slaveProcessDigitsCount);
        startTime = MPI_Wtime();
    }
 
    int error;

    int *dataToSend;
    int *dataToReceive = (int *)malloc(2 * sizeof(int));

    if (myrank == 0)
    {
        dataToSend = (int *)malloc(groupSize * 2 * sizeof(int));

        dataToSend[0] = 0;
        dataToSend[1] = masterProcessDigitsCount;

        int offset = masterProcessDigitsCount;
        for(int count = 1; count < groupSize; count++)
        {
            dataToSend[count * 2] = offset;
            dataToSend[count * 2 + 1] = slaveProcessDigitsCount;

            offset += slaveProcessDigitsCount;   
        }
    }

    error = MPI_Scatter((void *)dataToSend, 2, MPI_INT, (void *)dataToReceive, 2, MPI_INT, 0, comm);
    MPIerror(error, myrank);

    char *digits = (char *)malloc(sizeof(char) * dataToReceive[1]);
    for (int count = 0; count < dataToReceive[1]; count++)
    {
        digits[count] = calculatePiDigit(count + dataToReceive[0]);
    }

    char *result;
    if (myrank == 0)
    {
        result = (char *)malloc(sizeof(char) * digitsToCount);
        //memcpy(result, digits, masterProcessDigitsCount);
    }

    int *displacements = (int *)malloc(sizeof(int) * groupSize);
    int *receiveCounts = (int *)malloc(sizeof(int) * groupSize);

    displacements[0] = 0;
    receiveCounts[0] = masterProcessDigitsCount;
    int offset = masterProcessDigitsCount;
    for (int count = 1; count < groupSize; count++)
    {
        receiveCounts[count] = slaveProcessDigitsCount;
        displacements[count] = offset;
        offset += slaveProcessDigitsCount;
    }

    error = MPI_Gatherv(digits, dataToReceive[1], MPI_CHAR, (void *)result, receiveCounts, displacements, MPI_CHAR, 0, comm);
    MPIerror(error, myrank);
    
    free(displacements);
    free(receiveCounts);

    if (myrank == 0) 
    {
        free(dataToSend);
        printf("Group #%d, processes count %d, time is %f\n\n", color, groupSize, MPI_Wtime() - startTime);

        if (color == 0)
        {
            printResult(result, digitsToCount);
        }

        free(result);
    }
    
    free(digits);
    free(dataToReceive);

    MPI_Finalize();
    return 0;
}

