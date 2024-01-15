/**
 * CP386 Assignent 5 Question 1
 * Shawn Phung 200814180
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 4

int available[NUM_RESOURCES];
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];
int need[NUM_CUSTOMERS][NUM_RESOURCES];
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];
int safe_sequence[NUM_CUSTOMERS];

// Read 
void read_max_resources(){
    // Open sample file
    FILE *file = fopen("sample_in_banker.txt", "r");
    if (file == NULL) {
        printf("Could not open file for reading\n");
        exit(-1);
    }
    // Iterate through file
    for(int i = 0; i < NUM_CUSTOMERS; i++)
        for(int j = 0; j < NUM_RESOURCES; j++){
            if(j == NUM_RESOURCES-1)
                fscanf(file, "%d", &maximum[i][j]);
            else
                fscanf(file, "%d,", &maximum[i][j]);
        }
    fclose(file);
}

// Status  -> print
void print_state(){
    printf("Available Resources:\n");
    for(int i = 0; i < NUM_RESOURCES; i++)
        printf("%d ", available[i]);
    printf("\n");
    
    printf("Maximum Resources:\n");
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        for(int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", maximum[i][j]);
        printf("\n");
    }

    printf("Allocated Resources:\n");
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        for(int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", allocation[i][j]);
        printf("\n");
    }
    printf("Needed Resources:\n");
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        for(int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }
}

// Release
void release_resources(char *command){
    int cust_num, res;
    char *token = strtok(command, " ");
    token = strtok(NULL, " ");
    cust_num = atoi(token);

    for(int i = 0; i < NUM_RESOURCES; i++){
        token = strtok(NULL, " ");
        res = atoi(token);
        if(res > allocation[cust_num][i]){
            printf("Cannot release more than allocated resources\n");
            return;
        }
        allocation[cust_num][i] -= res;
        available[i] += res;
    }
    printf("The resources have been released successfully\n");
}

// Request
void request_resources(char *command){
    int cust_num;
    char *token = strtok(command, " ");
    token = strtok(NULL, " ");
    cust_num = atoi(token);

    int temp_avail[NUM_RESOURCES];
    for(int i = 0; i < NUM_RESOURCES; i++){
        temp_avail[i] = available[i];
    }

    for(int i = 0; i < NUM_RESOURCES; i++){
        token = strtok(NULL, " ");
        int res = atoi(token); 
        if(res > maximum[cust_num][i]){
            printf("Cannot request more than maximum resources\n");
            return;
        }
        temp_avail[i] -= res;
        if (temp_avail[i] < 0){
            printf("Not enough resources available\n");
            return;
        }

        allocation[cust_num][i] += res;
        available[i] = temp_avail[i];
        need[cust_num][i] = maximum[cust_num][i] - allocation[cust_num][i];
    }

    printf("State is safe, and request is satisfied\n");
}

// Safe state
int is_safe() {
    int work[NUM_RESOURCES];
    int finish[NUM_CUSTOMERS] = {0};

    for(int i = 0; i < NUM_RESOURCES; i++) {
        work[i] = available[i];
    }

    int index = 0;
    while(index < NUM_CUSTOMERS) {
        int found = 0;
        for(int i = 0; i < NUM_CUSTOMERS; i++) {
            if (finish[i] == 0) {
                int j;
                for(j = 0; j < NUM_RESOURCES; j++) {
                    if(need[i][j] > work[j]) {
                        break;
                    }
                }
                if(j == NUM_RESOURCES) {
                    for(int k = 0; k < NUM_RESOURCES; k++) {
                        work[k] += allocation[i][k];
                    }
                    safe_sequence[index++] = i;
                    finish[i] = 1;
                    found = 1;
                }
            }
        }
        if(found == 0) {
            return 0;
        }
    }
    return 1;
}

// Run
void run() {
    if(is_safe()) {
        printf("Safe Sequence is: ");
        for(int i = 0; i < NUM_CUSTOMERS; i++){
            printf("%d ", safe_sequence[i]);
        }
        printf("\n");

        for(int s = 0; s < NUM_CUSTOMERS; s++){
            int i = safe_sequence[s]; 
            printf("--> Customer/Thread %d\n", i);
            printf("    Allocated resources: ");
            for(int j = 0; j < NUM_RESOURCES; j++){
                printf("%d ", allocation[i][j]);
            }
            printf("\n    Needed: ");
            for(int j = 0; j < NUM_RESOURCES; j++){
                printf("%d ", need[i][j]);
            }
            printf("\n    Available: ");
            for(int j = 0; j < NUM_RESOURCES; j++){
                printf("%d ", available[j]);
            }
            printf("\n    Thread has started\n");
            printf("    Thread has finished\n");
            printf("    Thread is releasing resources\n");
            for(int j = 0; j < NUM_RESOURCES; j++){
                available[j] += allocation[i][j];
                allocation[i][j] = 0;
                need[i][j] = maximum[i][j];
            }
            printf("    New Available: ");
            for(int j = 0; j < NUM_RESOURCES; j++){
                printf("%d ", available[j]);
            }
            printf("\n");
        }
    }
    else {
        printf("The system is not in a safe state!\n");
    }
}

int main(int argc, char *argv[]){
    if(argc != NUM_RESOURCES+1){
        printf("Please specify available resources as command line arguments\n");
        exit(-1);
    }

    for(int i = 0; i < NUM_RESOURCES; i++)
        available[i] = atoi(argv[i+1]);

    printf("Number of Customers: %d\n", NUM_CUSTOMERS);
    printf("Currently Available resources: ");
    for(int i = 0; i < NUM_RESOURCES; i++)
        printf("%d ", available[i]);
    printf("\n");
    
    read_max_resources();

    printf("Maximum resources from file:\n");
    for(int i = 0; i < NUM_CUSTOMERS; i++){
        for(int j = 0; j < NUM_RESOURCES; j++)
            printf("%d ", maximum[i][j]);
        printf("\n");
    }
    
    printf("Enter Command: ");
    fflush(stdout);
    char command[256];

    while(fgets(command, sizeof(command), stdin)){
        
command[strcspn(command, "\n")] = 0;
        // Request
        if(strncmp(command, "RQ", 2) == 0)
            request_resources(command);
        // Release
        else if(strncmp(command, "RL", 2) == 0)
            release_resources(command);
        // Status
        else if(strncmp(command, "Status", 6) == 0)
            print_state();
        // Run
        else if(strncmp(command, "Run", 3) == 0)
            run();
        // Exit
        else if(strncmp(command, "Exit", 4) == 0)
            break;
        else printf("Invalid input, use one of RQ, RL, Status, Run, Exit\n");
        
        printf("Enter Command: ");
        fflush(stdout);
    }

    return 0;
}