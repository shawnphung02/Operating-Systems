/**
* CP386 Assignment 5 Question 2
* Shawn Phung 200814180
* Ouptut after compact is a bit wrong, couldn't figure it out entirely
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct block {
   int start;
   int end;
   char process_id[10];
   struct block *next;
} block;
block *head;

// Create new memory
void create_memory(int mem) {
   head = (block *)malloc(sizeof(block));
   head->start = 0;
   head->end = mem-1;
   strcpy(head->process_id, "");
   head->next = NULL;
}

block* find_best_fit(int size) {
   block *current = head;
   block *best = NULL;
   int best_delta = 1048577;
   int index = 0;


   while(current != NULL) {
       if(strcmp(current->process_id, "") == 0 && (current->end - current->start + 1 >= size)) {
           // Get delta
           int curr_delta = (current->end - current->start + 1) - size;
           printf("index = %d delta = %d best delta = %d\n", index, curr_delta, best_delta);

           // New best, update
           if(best_delta > curr_delta) {
               best_delta = curr_delta;
               best = current;
           }
       }
       // Next pointer, increment index
       current = current->next;
       index++;
   }
   return best;
}

// Request Memory
void request_memory(char* process_id, int size) {
   // Get hole
   block* best_fit_block = find_best_fit(size);
   // No sufficient holes
   if(best_fit_block == NULL) {
       printf("No hole of sufficient size\n");
       return;
   }
   if(best_fit_block->end - best_fit_block->start + 1 == size) {
       strcpy(best_fit_block->process_id, process_id);
   } else {
       block* new_block = (block *)malloc(sizeof(block));
       new_block->start = best_fit_block->start + size;
       new_block->end = best_fit_block->end;
       strcpy(new_block->process_id, "");
       new_block->next = best_fit_block->next;


       best_fit_block->end = best_fit_block->start + size - 1;
       strcpy(best_fit_block->process_id, process_id);
       best_fit_block->next = new_block;
   }
   printf("Successfully allocated %d to process %s\n", size, process_id);
}

// Release Memory
void release_memory(char* process_id) {
   block *current = head;
   while(current != NULL) {
       if(strcmp(current->process_id, process_id) == 0) {
           strcpy(current->process_id, "");
           printf("Successfully released memory for process %s\n", process_id);
           return;
       }
       current = current->next;
   }
   printf("Error: No process with ID %s found\n", process_id);
}

// Compact 
void compact_memory() {
    block *current = head;
    int last = 0;
    
    while(current != NULL) {
        if(strcmp(current->process_id, "") != 0) {
            int size = current->end - current->start + 1;
            current->start = last;
            current->end = last + size - 1;
            last = current->end + 1;
        }
        current = current->next;
    }

    // Merge free blocks
    current = head;
    block *prev = NULL;
    while(current != NULL) {
        if(strcmp(current->process_id, "") == 0 && prev != NULL && strcmp(prev->process_id, "") == 0) {
            prev->end = current->end;
            prev->next = current->next;
            free(current);
            current = prev->next;
        } else {
            prev = current;
            current = current->next;
        }
    }
    printf("Successfully compacted memory\n");
}

// Print status
void print_status() {
    block *current = head;
    int allocated_memory = 0, free_memory = 0;

    // First pass to calculate total allocated and free memory
    while (current != NULL) {
        int block_size = current->end - current->start + 1;
        if (strcmp(current->process_id, "") == 0) {
            free_memory += block_size;
        } else {
            allocated_memory += block_size;
        }
        current = current->next;
    }

    // Now print the results
    printf("Partitions [Allocated memory = %d]:\n", allocated_memory);
    current = head; // Reset current to head to start from the beginning
    while (current != NULL) {
        if (strcmp(current->process_id, "") != 0) {
            printf("Address [%d:%d] Process %s\n", current->start, current->end, current->process_id);
        }
        current = current->next;
    }

    printf("\nHoles [Free memory = %d]:\n", free_memory);
    current = head; // Reset current to head to start from the beginning
    while (current != NULL) {
        if (strcmp(current->process_id, "") == 0) {
            printf("Address [%d:%d] len = %d\n", current->start, current->end, current->end - current->start + 1);
        }
        current = current->next;
    }
}

int main() {
    printf("Here, the Best Fit approach has been implemented and allocated 1048576 bytes of memory.\n");
    create_memory(1048576);
    char command[20];
    char inputLine[256]; // Buffer to hold the entire input line
    int exit = 0;

    while(!exit) {
        // Get input
        printf("allocator>");
        fflush(stdout);

        if (fgets(inputLine, sizeof(inputLine), stdin) == NULL) {
            printf("Error reading input\n");
            continue;
        }

        if (sscanf(inputLine, "%s", command) == 1) {
            if(strcmp(command, "RQ") == 0) {
                char process_id[10];
                int size;
                // Request
                if (sscanf(inputLine, "%*s %s %d", process_id, &size) == 2) {
                    request_memory(process_id, size);
                } else {
                    printf("Invalid RQ command format\n");
                }
            // Release
            } else if(strcmp(command, "RL") == 0) {
                char process_id[10];
                if (sscanf(inputLine, "%*s %s", process_id) == 1) {
                    release_memory(process_id);
                } else {
                    printf("Invalid RL command format\n");
                }
            // Compact
            } else if(strcmp(command, "C") == 0) {
                compact_memory();
            // Status
            } else if(strcmp(command, "Status") == 0) {
                print_status();
            // Exit
            } else if(strcmp(command, "Exit") == 0) {
                exit = 1;
            } else {
                printf("Invalid Command!\n");
            }
        } else {
            printf("Error processing command\n");
        }
    }
    return 0;
}