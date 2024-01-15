#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

// Keep calculating n until 1 reached
void collatz(int n, int *shm) {
    while(n != 1) {
        *shm = n;
        shm++;
        // Case 1
        if (n % 2 == 0) {
            n /= 2;
        // Case 2
        } else {
            n = 3 * n + 1;
        }
    }
    // End reached
    *shm = 1;
}

int estimate_collatz_length(int n) {
    // Overestimate the collatz length 
    return 3 * log2(n) + 1;  
}

int get_line_count(FILE *file) {
    int line_count = 0;
    char ch;
    // Get file size
    while(!feof(file)) {
        ch = fgetc(file);
        if(ch == '\n') {
            // Terminating \n found, increment line count
            line_count++;
        }
    }
    rewind(file);  // Reset the file pointer to the beginning
    return line_count;
}

int main() {
    FILE *fp = fopen("start_numbers.txt", "r");
    // Check if able to open file
    if (!fp) {
        perror("Could not open file: start_numbers.txt");
        exit(1);
    }
    // Get line count
    int line_count = get_line_count(fp);
    int *numbers = malloc(sizeof(int) * line_count);
    // Check for error in memory allocation
    if (!numbers) {
        perror("Memory allocation failed");
        exit(1);
    }

    // Scan through file and append numbers to an array
    for (int i = 0; i < line_count; i++) {
        fscanf(fp, "%d", &numbers[i]);
    }
    fclose(fp);

    // Calculate total size
    int size = 0;
    for (int i = 0; i < line_count; i++) {
        size += estimate_collatz_length(numbers[i]);
    }

    // Create shared memory 
    int fd = shm_open("collatz_seq", O_CREAT | O_RDWR, 0666);
    // Set size to memory segment
    ftruncate(fd, sizeof(int) * size);
    
    // Map memory segment to shm
    int *shm = mmap(0, sizeof(int) * size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // Check for errors in mapping memory segment
    if(shm == MAP_FAILED) {
        perror("Unable to map.");
        exit(1);
    }
        // Iterate through input array
       for (int i = 0; i < line_count; i++) {
        // Get estimated length of index value
        int estimated_length = estimate_collatz_length(numbers[i]);

        // Create new child process
        pid_t pid = fork();
        // Error in fork() system call
        if (pid < 0) {
            perror("Unable to fork");
            exit(1);
        }

        if (pid == 0) {  // In child process
            collatz(numbers[i], shm);
            printf("Child Process: The generated collatz sequence is ");
            // Iterate through collatz sequence
            int *ptr = shm;
            while (*ptr != 1) {
                // Print calculated value
                printf("%d ", *ptr);
                ptr++;
            }
            // End reached, print 1
            printf("1\n");
            exit(0);
        } else {  // In parent process
            // Print inpiut number
            printf("Parent Process: The positive integer read from file is %d\n", numbers[i]);
            wait(NULL);  
            // Adjust memory segment size to estimated length
            shm += estimated_length;
        }
    }
    // End reached, reset everything and free memory
    munmap(shm, sizeof(int) * size);
    shm_unlink("collatz_seq");
    free(numbers); 

    return 0;
}
