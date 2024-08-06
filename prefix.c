//////////////////////////////
// Author: James            //
//          CW 1            //
//////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include <time.h> //Used to set seed for generating random numbers
#include "mpi.h"  //Needed for MPI
#include "math.h" //Needed for pow, log2, floor and ciel.

//**********************************//
//      Function Definition:        //
//          paddingCalc             //
//**********************************//
// Input arguments      : The value of n (size of array), and the value of p (number of processes)
// Return value         : Value to pad an array to
// Required header files: N/A
// Warnings             : No Input sanitising, assumed to be done before function call.

int paddingCalc(int n, int numProc)
{
    int multipleOfProc = numProc; // We will pad to the nearest multiple of the number of processes
    int i = 2;
    while (n > multipleOfProc)
    {
        multipleOfProc = numProc * i; // Increment n in multiples of numProc until n is larger than numProc
        i++;
    }
    return (multipleOfProc);
}

//**********************************//
//      Function Definition:        //
//              upPhase             //
//**********************************//
// Input arguments      : The maximum prefix scan 'level', The number of values per process, The process rank, The number of processes, The data array
// Return value         : N/A
// Required header files: "math.h", "MPI.h"
// Warnings             : No Input sanitising, assumed to be done before function call.

void upPhase(int maximumLevel, int valuesPerProc, int rank, int numProcs, int *procDataInput)
{
    for (int level = 2; level <= maximumLevel; level *= 2) // Iterates through each level on the up phase
    {
        // Variables for Sending and Receiving
        int sendPartner;
        int receivePartner;

        for (int i = 0; i < valuesPerProc; i++) // iterates through each value in the process specific array
        {
            int originalArrayPos = (rank * valuesPerProc) + i + 1; // This is essentially the original position in the input array (Starting from 1).
                                                          // It allows us to work out what should happen to each value (I.e. Send Receive etc.)

            if (originalArrayPos % level == 0) // If True then receiver for this level
            {
                receivePartner = floor((((originalArrayPos-1) - (level / 2)) / valuesPerProc)); // Works out which rank holds the value that the current process should receive from.

                if (receivePartner == rank) // If the process that should receive the data is expecting the data from itself then add the value to be 'sent' to the current value
                {
                    procDataInput[i] = procDataInput[i] + procDataInput[i - level / 2];
                }
                else // If the value is to be received from another process
                {
                    int recvBuff;
                    MPI_Recv(&recvBuff, 1, MPI_INT, receivePartner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Wait for incoming data from the other process
                    procDataInput[i] = procDataInput[i] + recvBuff;                                        // Add the received data to the current position in the array.
                }
            }
            else if (abs((originalArrayPos - level) % level) == level / 2) // if True then sender for this level
            {
                sendPartner = floor((((originalArrayPos-1) + level / 2) / valuesPerProc)); // Works out which rank needs the value in the array that this process is currently dealing with
                if (sendPartner == rank)                                          // If the value needs to be sent to the same process that is dealing with it
                {
                    // Do nothing as will be handled in the 'receive' condition
                }
                else if (sendPartner < numProcs) // Make sure that we are not sending to a process that doesn't exist
                {
                    MPI_Send(&procDataInput[i], 1, MPI_INT, sendPartner, 0, MPI_COMM_WORLD); // Send the data
                }
            }
            // If neither: Do Nothing as not a sender or receiver for this level
        }
    }
}

//**********************************//
//      Function Definition:        //
//          downPhase               //
//**********************************//
// Input arguments      : The maximum prefix scan 'level', The number of values per process, The process rank, The number of processes, The data array
// Return value         : N/A
// Required header files: "math.h", "MPI.h"
// Warnings             : No Input sanitising, assumed to be done before function call.
void downPhase(int maximumLevel, int valuesPerProc, int rank, int numProcs, int *procDataInput)
{
    for (int level = maximumLevel / 2; level >= 2; level /= 2) // iterates through each level on the up phase
    {
        // Variables for Sending and Receiving
        int sendPartner;
        int receivePartner;

        for (int i = 0; i < valuesPerProc; i++)     // iterates through each value in the process specific array
        {
            int originalArrayPos = (rank * valuesPerProc) + i + 1;   // This is essentially the original position in the input array (Starting at 1)
                                                            // It allows us to work out what should happen to each value (I.e. Send Receive etc.)

            if (((originalArrayPos - level) % level) == level / 2) // If True then receiver for this level
            {
                receivePartner = floor((((originalArrayPos-1) - (level / 2)) / valuesPerProc)); // Works out which rank holds the value that the current process should receive from.

                if (receivePartner == rank) // If the process that should receive the data is expecting the data from itself then add the value to be 'sent' to the current value
                {
                    procDataInput[i] = procDataInput[i] + procDataInput[i - level / 2];
                }
                else
                {
                    int recvBuff;
                    MPI_Recv(&recvBuff, 1, MPI_INT, receivePartner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // Wait for incoming data from the other process
                    procDataInput[i] = procDataInput[i] + recvBuff;                                         // Add the received data to the current position in the array.
                }
            }
            else if (originalArrayPos % level == 0) // If True then sender for this level
            {
                sendPartner = floor((((originalArrayPos-1) + level / 2) / valuesPerProc)); // Works out which rank needs the value in the array that this process is currently dealing with

                if (sendPartner == rank) // If the value needs to be sent to the same process that is dealing with it
                {
                    // Do nothing as will be handled in the 'receive' condition
                }

                else if (sendPartner < numProcs) // Make sure that we are not sending to a process that doesn't exist
                {
                    MPI_Send(&procDataInput[i], 1, MPI_INT, sendPartner, 0, MPI_COMM_WORLD); // Send the data
                }
            }
            // If neither: Do Nothing as not a sender or receiver for this level
        }
    }
}

//**********************************//
//      Main Definition:            //
//**********************************//
int main(int argc, char *argv[])
{
    // Initialising MPI
    MPI_Init(&argc, &argv);

    // Getting useful values about the MPI environment
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    //**********************************//
    //          USER INPUT              //

    int n;           // Variable which stores user specified size of prefix scan (Only stored on process 0)
    int *inputArray; // Array to store the random integers
    int paddedSize;  // variable to store the padded array size (Will be broadcast to all processes)

    if (rank == 0) // User input on Process 0 only
    {
        // Input Sanitising
        printf("Please Enter a Value for n \n");
        fflush(stdout);
        while (scanf("%d", &n) != 1 // fails if don't enter a legitimate integer
               || n <= 0 || n == 1) // fails if nonsensical integer value
        {

            fflush(stdin); // The Illegal data entered needs to be "flushed"

            printf("Illegal value entered: Must be an integer greater than 1\n"); // Notify User
            printf("Please Enter a Value for n \n");
            fflush(stdout);
        }

        paddedSize = paddingCalc(n, numProcs); // Dynamically allocate memory for the array based on the the padded array size

        inputArray = (int *)calloc(paddedSize, sizeof(int)); // Calloc Initialises the memory to zeros so does the padding for us.
        if (inputArray == NULL)                              // Checking for memory allocation error
        {
            printf("Calloc returned NULL\n");
            exit(1);
        }

        //Seed the random number generator with the current time
        srand(time(NULL));

        // Generate and store random integers
        for (int i = 0; i < n; i++)
        {
            inputArray[i] = rand() % 1000; //Want to limit the maximum random value as there is chance of rand() returning INT_MAX
        }
    }
    MPI_Barrier(MPI_COMM_WORLD); // A process proceeding past here before process 0 has allocated the arrays could cause problems
    //           END USER INPUT         //
    //**********************************//

    //**********************************//
    //         Process Variables       //
    MPI_Bcast(&paddedSize, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcasting the "paddedSize" to all processes

    // Declaring Variables for all processes
    int valuesPerProc = paddedSize / numProcs; // Will always be whole number as paddedSize is always a multiple of numProcs
    int procDataInput[valuesPerProc];          // Array for each process for Scatter and Gather

    int log2Array = ceil(log2(paddedSize)); // Calculating the maximum 'level' for the UP/Down phase
    int maximumLevel = pow(2, log2Array);

    MPI_Barrier(MPI_COMM_WORLD);
    //      End Process Variables       //
    //**********************************//



    //**********************************//
    //         Perform Prefix Scan      //

    // Scatter - To All processes
    MPI_Scatter(inputArray, valuesPerProc, MPI_INT, procDataInput, valuesPerProc, MPI_INT, 0, MPI_COMM_WORLD);

    // UP PHASE
    upPhase(maximumLevel, valuesPerProc, rank, numProcs, procDataInput);

    // Down Phase
    downPhase(maximumLevel, valuesPerProc, rank, numProcs, procDataInput);

    int outputArray[paddedSize]; // Create an output array big enough to hold all the data from all the processes
    
    // Gather - From all processes
    MPI_Gather(procDataInput, valuesPerProc, MPI_INT, outputArray, valuesPerProc, MPI_INT, 0, MPI_COMM_WORLD);

    //          End Prefix Scan         //
    //**********************************//


    //**********************************//
    // Printing Final Solution      //

    if (rank == 0)
    {
        // Print the original unpadded array
        printf("Original Unpadded Array:    ");
        for (int i = 0; i < n; i++)
        {
            printf("%d  ", inputArray[i]);
        }
        printf("\n");
        fflush(stdout);

        // Print the output array
        printf("Output Array:               ");
        for (int i = 0; i < n; i++) // Only print the non padded values
        {
            printf("%d  ", outputArray[i]);
        }
        printf("\n");
        fflush(stdout);
        // Free the dynamically allocated memory
        free(inputArray);
    }

    MPI_Finalize(); //End the MPI communications   
    return 0;
}
