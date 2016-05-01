#include "stuff.h"


int main(int argc, char *argv[]) 
{
    int myrank, nodesCount;
    
    MPI_Init(&argc, &argv);
  
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nodesCount);  

    
    int digitsCount = atoi(argv[1]);

    int digitsToProcess = digitsCount / nodesCount;
    int digitsToLastProcess = digitsCount - ((nodesCount - 1) * digitsToProcess);

    if (myrank == 0)
    {
        //printf("%d processes\n %d digits count\n %d to all\n %d to last\n", nodesCount, digitsCount, digitsToProcess, digitsToLastProcess);
    }
    
    double startTime;  
    if (myrank == 0) 
    {
        startTime = MPI_Wtime();
    }

    int error;
    
    int offset;
    int length;

    if(myrank == 0)
    {
        int dataToSend[2];
        for(int count = 1; count < nodesCount; count++)
        {
            dataToSend[0] = count * digitsToProcess;
            dataToSend[1] = (count == nodesCount - 1) ? digitsToLastProcess : digitsToProcess;

            error = MPI_Send(dataToSend, 2, MPI_INT, count, TAG, MPI_COMM_WORLD);           
            MPIerror(error, myrank);
        }       

        offset = 0;
        length = digitsToProcess;
    }
    else   
    {
        int dataToReceive[2];
        error = MPI_Recv(dataToReceive, 2, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPIerror(error, myrank);

        offset = dataToReceive[0];
        length = dataToReceive[1];
    }
    
    char *digits = (char *)malloc(sizeof(char) * length);

    for (int count = 0; count < length; count++)
    {
        digits[count] = calculatePiDigit(count + offset);
    }
      
    if (myrank == 0)
    {
        char *result = (char *)malloc((digitsCount) * sizeof(char));

        memcpy(result, digits, length);

        for(int count = 1; count < nodesCount; count++)
        {
            error = MPI_Recv((void *)(result + count * digitsToProcess), (count == nodesCount - 1) ? digitsToLastProcess : digitsToProcess, MPI_CHAR, count, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPIerror(error, myrank);
        }         

        printf("time %f\n", MPI_Wtime() - startTime);
        printResult(result, digitsCount);
        

        free(result);
    }
    else 
    {
        error = MPI_Send(digits, sizeof(char) * length, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
        MPIerror(error, myrank);
    }
    
    free(digits);
    
    MPI_Finalize();
    return 0;
}
