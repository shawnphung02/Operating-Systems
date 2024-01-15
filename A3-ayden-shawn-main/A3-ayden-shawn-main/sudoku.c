#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 27

// Global array that threads can update to signal if their checks pass
int valid_checks[NUM_THREADS] = {0};

// Parameter structure for passing to the worker threads
typedef struct {
    int row;
    int column;
} parameters;

// Sudoku puzzle to be validated
int sudoku_puzzle[9][9] = {
    {5, 3, 4, 6, 7, 8, 9, 1, 2},
    {6, 7, 2, 1, 9, 5, 3, 4, 8},
    {1, 9, 8, 3, 4, 2, 5, 6, 7},
    {8, 5, 9, 7, 6, 1, 4, 2, 3},
    {4, 2, 6, 8, 5, 3, 7, 9, 1},
    {7, 1, 3, 9, 2, 4, 8, 5, 6},
    {9, 6, 1, 5, 3, 7, 2, 8, 4},
    {2, 8, 7, 4, 1, 9, 6, 3, 5},
    {3, 4, 5, 2, 8, 6, 1, 7, 9}
};

// Worker function to validate columns
void *validate_column(void* arg) {
    parameters *param = (parameters*) arg;
    int col = param->column;
    if (param->row != 0 || col > 8) {
        fprintf(stderr, "Column validation received invalid parameters.\n");
        pthread_exit(NULL);
    }

    // Validation array for checking the occurrence of numbers
    int seen[9] = {0};
    for (int row = 0; row < 9; row++) {
        int num = sudoku_puzzle[row][col];
        if (num < 1 || num > 9 || seen[num - 1] == 1) {
            pthread_exit(NULL);
        } else {
            seen[num - 1] = 1;
        }
    }
    valid_checks[18 + col] = 1; // Flag this column as valid
    pthread_exit(NULL);
}

// Worker function to validate rows
void *validate_row(void* arg) {
    parameters *param = (parameters*) arg;
    int row = param->row;
    if (param->column != 0 || row > 8) {
        fprintf(stderr, "Row validation received invalid parameters.\n");
        pthread_exit(NULL);
    }

    int seen[9] = {0};
    for (int col = 0; col < 9; col++) {
        int num = sudoku_puzzle[row][col];
        if (num < 1 || num > 9 || seen[num - 1] == 1) {
            pthread_exit(NULL);
        } else {
            seen[num - 1] = 1;
        }
    }
    valid_checks[9 + row] = 1; // Flag this row as valid
    pthread_exit(NULL);
}

// Worker function to validate 3x3 subsections
void *validate_subsection(void* arg) {
    parameters *param = (parameters*) arg;
    int startRow = param->row;
    int startCol = param->column;
    if (startRow > 6 || startRow % 3 != 0 || startCol > 6 || startCol % 3 != 0) {
        fprintf(stderr, "3x3 subsection validation received invalid parameters.\n");
        pthread_exit(NULL);
    }

    int seen[9] = {0};
    for (int i = startRow; i < startRow + 3; i++) {
        for (int j = startCol; j < startCol + 3; j++) {
            int num = sudoku_puzzle[i][j];
            if (num < 1 || num > 9 || seen[num - 1] == 1) {
                pthread_exit(NULL);
            } else {
                seen[num - 1] = 1;
            }
        }
    }
    valid_checks[startRow + startCol / 3] = 1; // Map the subsection to an index in the valid_checks array
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int index = 0;
    int i,j;

    // Create 9 threads for 9 3x3 subsections, 9 threads for 9 columns and 9 threads for 9 rows.
    // This will end up with a total of 27 threads.
     for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            // Check for starting index of each 3x3 subsection
            if (i % 3 == 0 && j % 3 == 0) {
                parameters *data_subsection = (parameters *) malloc(sizeof(parameters));
                data_subsection->row = i;
                data_subsection->column = j;
                pthread_create(&threads[index++], NULL, validate_subsection, data_subsection);
            }
            // Check for each column
            if (i == 0) {
                parameters *data_column = (parameters *) malloc(sizeof(parameters));
                data_column->row = i;
                data_column->column = j;
                pthread_create(&threads[index++], NULL, validate_column, data_column);
            }
            // Check for each row
            if (j == 0) {
                parameters *data_row = (parameters *) malloc(sizeof(parameters));
                data_row->row = i;
                data_row->column = j;
                pthread_create(&threads[index++], NULL, validate_row, data_row);
            }
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Check the results of the validations
    for (int i = 0; i < NUM_THREADS; i++) {
        if (valid_checks[i] == 0) {
            printf("Sudoku solution is not valid.\n");
            return EXIT_FAILURE; // Return failure if any validation failed
        }
    }

    printf("Sudoku solution is valid.\n");
    return EXIT_SUCCESS; // Return success if all validations passed
}