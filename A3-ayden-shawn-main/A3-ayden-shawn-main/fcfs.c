#include <stdio.h>

// Define structure to store thread scheduling information
struct ThreadInfo {
    int p_id;                // Process ID
    int arr_time;            // Arrival time
    int burst_time;          // Burst time (time required to complete)
    int turn_around_time;    // Turn-around time (completion time - arrival time)
    int waiting_time;        // Waiting time (turn-around time - burst time)
    int completion_time;     // Completion time (time at which process completes)
};

int main() {
    // Attempt to open the input file for reading
    FILE *inputFile = fopen("sample_in_schedule.txt", "r");
    // If the file cannot be opened, print an error message and exit
    if (inputFile == NULL) {
        perror("Error opening file");
        return 1;
    }

    int numThreads = 0;
    // Read the number of threads from the file
    fscanf(inputFile, "%d", &numThreads);

    // Allocate an array of ThreadInfo structures based on the number of threads
    struct ThreadInfo threads[numThreads];

    // Loop to read each thread's info from the file
    for (int i = 0; i < numThreads; i++) {
        fscanf(inputFile, "%d,%d,%d", &threads[i].p_id, &threads[i].arr_time, &threads[i].burst_time);
    }

    // Close the file as we're done reading from it
    fclose(inputFile);

    // Initialize the current time to zero before we start scheduling
    int current_time = 0;
    // Iterate over threads to calculate scheduling times
    for (int i = 0; i < numThreads; i++) {
        // If the current time is before the thread's arrival, fast-forward to arrival time
        if (current_time < threads[i].arr_time) {
            current_time = threads[i].arr_time;
        }
        // Set the thread's completion time
        threads[i].completion_time = current_time + threads[i].burst_time;

        // Calculate turn-around time for the thread
        threads[i].turn_around_time = threads[i].completion_time - threads[i].arr_time;
        // Calculate waiting time for the thread
        threads[i].waiting_time = threads[i].turn_around_time - threads[i].burst_time;

        // Update the current time to the completion time of the current thread
        current_time = threads[i].completion_time;
    }

    // Variables to store the sum of all waiting times and turn-around times
    double avgwaiting_time = 0.0;
    double avgturn_around_time = 0.0;

    // Loop to accumulate waiting and turn-around times
    for (int i = 0; i < numThreads; i++) {
        avgwaiting_time += threads[i].waiting_time;
        avgturn_around_time += threads[i].turn_around_time;
    }

    // Calculate the average waiting time
    avgwaiting_time /= numThreads;
    // Calculate the average turn-around time
    avgturn_around_time /= numThreads;

    // Print a header for the results table
    printf("Thread ID\tArrival Time\tBurst Time\tCompletion Time\tTurn-Around Time\tWaiting Time\n");
    // Loop to print each thread's scheduling information
    for (int i = 0; i < numThreads; i++) {
        printf("%d\t\t%d\t\t%d\t\t%d\t\t\t%d\t\t\t%d\n", 
               threads[i].p_id, 
               threads[i].arr_time, 
               threads[i].burst_time, 
               threads[i].completion_time, 
               threads[i].turn_around_time, 
               threads[i].waiting_time);
    }

    // Print the average waiting time and average turn-around time
    printf("\nAverage Waiting Time: %.2lf\n", avgwaiting_time);
    printf("Average Turn-Around Time: %.2lf\n", avgturn_around_time);

    return 0; // Exit the program
}