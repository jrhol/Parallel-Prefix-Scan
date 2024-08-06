#include <stdio.h>
#include <stdlib.h>
#include <time.h> //Used to set seed for generating random numbers
#include "mpi.h"  //Needed for MPI
#include "math.h"

void prefix_scan(int n, int a[], int b[])
{
    b[0] = a[0];
    for (int i = 1; i < n; ++i)
        b[i] = b[i - 1] + a[i];
}

int main(int argc, char *argv[])
{
    // Initialising MPI
    MPI_Init(&argc, &argv);

    // Getting useful values about the MPI environment
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0)
    {
        int n = 6;
        int a[n];
        int b[n];

        for (int i = 0; i < n; i++)
        {
            a[i] = i + 1;
        }

        srand(756);

        // Generate and store random integers
        for (int i = 0; i < n; i++)
        {
            a[i] = rand() % 1000; //Want to limit the maximum random value as there is chance of rand() returning INT_MAX
        }

        prefix_scan(n, a, b);

        // Print the original input array
        printf("Input Array was:    ");
        for (int i = 0; i < n; i++)
        {
            printf("%d  ", a[i]);
        }
        printf("\n");
        fflush(stdout);

        // Print the output array
        printf("Output Array is:    ");
        for (int i = 0; i < n; i++) // Needs chaning to be the size of N, dont print the padded values
        {
            printf("%d  ", b[i]);
        }
    }

    MPI_Finalize(); //End the MPI communications               
}