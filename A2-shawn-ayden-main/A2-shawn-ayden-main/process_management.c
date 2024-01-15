#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

// Define constants for maximum command length and the shared memory key
#define MAX_CMD_LEN 1000
#define SHM_KEY_NUM 1234

// Function to write the output of a command to a file
void writeOutput(char *command, char *output) {
    FILE *fptr = fopen("output.txt", "a");
    if (fptr == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fprintf(fptr, "The output of: %s is\n", command);
    fprintf(fptr, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);
    fclose(fptr);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Create a shared memory segment
    int shmem_id = shmget(SHM_KEY_NUM, MAX_CMD_LEN, IPC_CREAT | 0666);
    if (shmem_id == -1) {
        perror("shmget");
        return 1;
    }

    // Fork a child process to read file contents
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return 1;
    } else if (child_pid == 0) {
        // Child process reads the file and writes to shared memory
        int shmem_fd = shmget(SHM_KEY_NUM, MAX_CMD_LEN, 0);
        if (shmem_fd == -1) {
            perror("shmget");
            return 1;
        }

        char *shmem_ptr = (char *)shmat(shmem_fd, NULL, 0);
        if (shmem_ptr == (char *)-1) {
            perror("shmat");
            return 1;
        }

        // Open the file for reading
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            perror("fopen");
            return 1;
        }

        // Read file contents into shared memory
        char command[MAX_CMD_LEN];
        while (fgets(command, MAX_CMD_LEN, file) != NULL) {
            strcat(shmem_ptr, command);
        }

        fclose(file);

        // Detach the shared memory segment
        if (shmdt(shmem_ptr) == -1) {
            perror("shmdt");
            return 1;
        }

        return 0;
    } else {
        // Parent process

        // Wait for the child process to finish reading the file
        wait(NULL);

        // Fork a child process to execute commands
        pid_t exec_child_pid = fork();
        if (exec_child_pid == -1) {
            perror("fork");
            return 1;
        } else if (exec_child_pid == 0) {
            // Child process executes commands and writes output to a pipe
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                return 1;
            }

            // Redirect stdout to the write end of the pipe
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            // Execute commands from shared memory
            int shmem_id = shmget(SHM_KEY_NUM, MAX_CMD_LEN, 0);
            if (shmem_id == -1) {
                perror("shmget");
                return 1;
            }

            char *shmem_ptr = (char *)shmat(shmem_id, NULL, 0);
            if (shmem_ptr == (char *)-1) {
                perror("shmat");
                return 1;
            }

            execl("/bin/sh", "sh", "-c", shmem_ptr, NULL);

            perror("execl");
            return 1;
        } else {
            // Parent process

            // Wait for the execution child process to finish
            wait(NULL);

            // Read output from the pipe
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                return 1;
            }

            // Redirect stdin to the read end of the pipe
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            // Open the output file for writing
            int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                perror("open");
                return 1;
            }

            // Read from the pipe and write to the output file
            char buffer[MAX_CMD_LEN];
            ssize_t bytes_read;
            while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
                if (write(output_fd, buffer, bytes_read) == -1) {
                    perror("write");
                    return 1;
                }
             
            }

            close(output_fd);
        }

        // Remove the shared memory segment
        if (shmctl(shmem_id, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            return 1;
        }

        return 0;
    }
}
