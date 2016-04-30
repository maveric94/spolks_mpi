#include "stuff.h"

char calculatePiDigit(int number)
{
    double pid, s1, s2, s3, s4;

    char chx[NHX];


/*  id is the digit position.  Digits generated follow immediately after id. */

    s1 = series (1, number);
    s2 = series (4, number);
    s3 = series (5, number);
    s4 = series (6, number);
    pid = 4. * s1 - 2. * s2 - s3 - s4;
    pid = pid - (int) pid + 1.;
    ihex (pid, NHX, chx);
  
    return chx[0];
}

void ihex (double x, int nhx, char chx[])

/*  This returns, in chx, the first nhx hex digits of the fraction of x. */

{
    int i;
    double y;
    char hx[] = "0123456789ABCDEF";

    y = fabs (x);

    for (i = 0; i < nhx; i++)
    {
        y = 16. * (y - floor (y));
        chx[i] = hx[(int) y];
    }
}

double series (int m, int id)

/*  This routine evaluates the series  sum_k 16^(id-k)/(8*k+m) 
    using the modular exponentiation technique. */

{
    int k;
    double ak, eps, p, s, t;
 


    s = 0.;

/*  Sum the series up to id. */

    for (k = 0; k < id; k++)
    {
        ak = 8 * k + m;
        p = id - k;
        t = expm (p, ak);
        s = s + t / ak;
        s = s - (int) s;
    }

/*  Compute a few terms where k >= id. */

    for (k = id; k <= id + 1000; k++)
    {
        ak = 8 * k + m;
        t = pow (16., (double) (id - k)) / ak;

        if (t < eps) 
            break;

        s = s + t;
        s = s - (int) s;
    }

    return s;
}

#define eps 1e-17

double expm (double p, double ak)

/*  expm = 16^p mod ak.  This routine uses the left-to-right binary 
    exponentiation scheme. */

{
    int i, j;
    double p1, pt, r;

    static double tp[ntp];
    static int tp1 = 0;

/*  If this is the first call to expm, fill the power of two table tp. */

    if (tp1 == 0) 
    {
        tp1 = 1;
        tp[0] = 1.;

        for (i = 1; i < ntp; i++) 
            tp[i] = 2. * tp[i-1];
    }

    if (ak == 1.) 
        return 0.;

/*  Find the greatest power of two less than or equal to p. */

    for (i = 0; i < ntp; i++) 
        if (tp[i] > p) 
            break;

    pt = tp[i-1];
    p1 = p;
    r = 1.;

/*  Perform binary exponentiation algorithm modulo ak. */

    for (j = 1; j <= i; j++)
    {
        if (p1 >= pt)
        {
            r = 16. * r;
            r = r - (int) (r / ak) * ak;
            p1 = p1 - pt;
        }

        pt = 0.5 * pt;
        if (pt >= 1.)
        {
            r = r * r;
            r = r - (int) (r / ak) * ak;
        }
    }

    return r;
}


void MPIerror(int error_code, int myrank)
{
    if(error_code != MPI_SUCCESS)
    {
        char error_string[BUFSIZ];
        int length_of_error_string;

        MPI_Error_string(error_code, error_string, &length_of_error_string);
        fprintf(stderr, "%3d: %s\n", myrank, error_string);
        exit(EXIT_FAILURE);
    }
}


void printResult(char *result, int length)
{
    printf("\n");
    for (int count = 0; count < length; count++)
    {
        if (count % 10 == 0)
        {
            if (count % 60)
            {
                printf(" ");
            }
            else
            {
                printf("\n");
            }
        }
        printf("%c", result[count]);

    }

    printf("\n");
}


int MPI_Rand_Split(int numberOfGroups, int rank, int numnodes, MPI_Comm* comm)
{
    srand(time(NULL));
    int* numbers = (int*)malloc(sizeof(int) * (numberOfGroups-1));
    
    for(int i = 0; i < numberOfGroups - 1; i++)
    {
        int num;
        while(1) 
        {
            num = rand_lim(numnodes - 1);
            if(num == 0)
                continue;
            
            bool isContain = false;
            for(int j = 0; j < i; j++)
            {
                if(numbers[j] == num)
                {
                    isContain = true;
                    break;
                }
            }
            if(isContain == false)
                break;
        }
        numbers[i] = num;
    }
    qsort (numbers, numberOfGroups-1, sizeof(int), compare);


    
    for(int i = 0; i < numberOfGroups - 1; i++)
    {         
        if(rank < numbers[i])
        {
            MPI_Comm_split( MPI_COMM_WORLD, i, rank, comm);
        
            free(numbers);

            return i;
        }
    }
    
    if(rank >= numbers[numberOfGroups - 2])
    {
        free(numbers);
       
        MPI_Comm_split( MPI_COMM_WORLD, numberOfGroups - 1, rank, comm);
        
        return numberOfGroups - 1;
    }

    printf("NOOOOOOOO\n");
}

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int rand_lim(int limit) 
{
/* return a random number between 0 and limit inclusive.
 */
    int divisor = RAND_MAX/(limit+1);
    int retval;

    do 
    { 
        retval = rand() / divisor;
    } 
    while (retval > limit);

    return retval;
}