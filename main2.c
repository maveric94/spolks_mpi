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

    int *groupIDs;

    if (myrank == 0)
    {
        if (color == 0)
        {
            groupIDs = (int *)malloc(sizeof(int) * (numberOfGroups - 1));
            int temp[2];

            for (int i = 0; i < numberOfGroups - 1; i++)
            {
                MPI_Recv(temp, 2, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                groupIDs[temp[0] - 1] = temp[1];

                //printf("color %d size %d\n", temp[0], temp[1]);
            }
        }
        else
        {
            int temp[2];
            temp[0] = color;
            temp[1] = groupSize;

            MPI_Send(temp, 2, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        }
    }


    int digitsPerGroup = digitsToCount / numberOfGroups;
    
    int slaveProcessDigitsCount = digitsPerGroup / groupSize;
    int masterProcessDigitsCount = digitsPerGroup - ((groupSize - 1) * slaveProcessDigitsCount);
    
    double startTime;  
    if (myrank == 0) 
    {
        //printf("master %d\n slave %d group %d perGroup %d\n", masterProcessDigitsCount, slaveProcessDigitsCount, groupSize, digitsPerGroup);
        startTime = MPI_Wtime();
    }
 
    int error;

    int *dataToSend;
    int *dataToReceive = (int *)malloc(2 * sizeof(int));

    if (myrank == 0)
    {
        dataToSend = (int *)malloc(groupSize * 2 * sizeof(int));

        int offset = digitsPerGroup * color;

        dataToSend[0] = offset;
        dataToSend[1] = masterProcessDigitsCount;

         printf("group %d calculating %d digits from %d offset\n", color,  dataToSend[1],  dataToSend[0]);
        
        for(int count = 1; count < groupSize; count++)
        {
            offset += slaveProcessDigitsCount;   

            dataToSend[count * 2] = offset;
            dataToSend[count * 2 + 1] = slaveProcessDigitsCount;

            printf("group %d calculating %d digits from %d offset\n", color,  dataToSend[count * 2 + 1],  dataToSend[count * 2]);
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
        result = (char *)malloc(sizeof(char) * digitsPerGroup);
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
        char *finalResult;
        if (color == 0)
        {         
            finalResult = (char *)malloc(sizeof(char) * digitsToCount);
            memcpy(finalResult, result, digitsPerGroup);
            int offset = groupSize;
            for (int count = 0; count < numberOfGroups - 1; count++)
            {
                MPI_Recv((void *)&finalResult[digitsPerGroup * (count + 1)], digitsPerGroup, MPI_CHAR, offset, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                offset += groupIDs[count];
            }
        }
        else
        {
            MPI_Send(result, digitsPerGroup, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
        }
        double workTime = MPI_Wtime() - startTime;
        printf("\ngroup %d process count %d time %f", color, groupSize, workTime);

        if (color == 0)
        {
            for (int i = 0; i < numberOfGroups - 1; i++)
            {
                double temp;
                MPI_Recv((void *)&temp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                workTime += temp;
            }

            printf("\n\ntotal time %f", workTime);

            printResult(finalResult, digitsToCount);
            free(finalResult);
        }
        else
        {
            MPI_Send((void *)&workTime, 1, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD);
        }

        free(result);
    }
    
    free(digits);
    free(dataToReceive);

    MPI_Finalize();
    return 0;
}

