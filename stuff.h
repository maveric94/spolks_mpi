#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <mpi.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>


#define NHX 16
#define ntp 25
#define TAG 20


double series (int m, int n);
void ihex (double x, int m, char c[]);
double expm (double x, double y);
char calculatePiDigit(int number);
void MPIerror(int error, int rank);
void printResult(char *result, int length);
int compare (const void * a, const void * b);
int rand_lim(int limit);
int MPI_Rand_Split(int numberOfGroups, int rank, int numnodes, MPI_Comm* comm);