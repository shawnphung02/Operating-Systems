#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include <semaphore.h>
#include <limits.h>

sem_t running;
sem_t even;
sem_t odd;

void logStart(char *tID); //function to log that a new thread is started
void logFinish(char *tID); //function to log that a thread has finished its time

void startClock(); //function to start program clock
long getCurrentTime(); //function to check current time since clock was started
time_t programClock; //the global timer/clock for the program

typedef struct thread //represents a single thread, you can add more members if required
{
    char tid[4]; //id of the thread as read from the file
    unsigned int startTime;
    int state;
    pthread_t handle;
    int retVal;
} Thread;

// Function prototypes
int threadsLeft(Thread *threads, int threadCount);
int threadToStart(Thread *threads, int threadCount);
void* threadRun(void *t); //the thread function, the code executed by each thread
int readFile(char *fileName, Thread **threads); //function to read the file content and build an array of threads
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Input file name missing...exiting with error code -1\n");
        return -1;
    }

    Thread *threads = NULL;
    int threadCount = readFile(argv[1], &threads);

    startClock();
    while (threadsLeft(threads, threadCount) > 0) {
        // Get the current time
        long currentTime = getCurrentTime();

        // Find and start threads with the same start time
        int i = threadToStart(threads, threadCount);
        if (i > -1) {
            threads[i].state = 1;
            threads[i].retVal = pthread_create(&(threads[i].handle), NULL,
                    threadRun, &threads[i]);
        }

        // Sleep for a short duration to avoid busy-waiting
        usleep(1000);
    }

    // Wait for all threads to finish
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i].handle, NULL);
    }

    return 0;
}


int readFile(char *fileName, Thread **threads) //do not modify this method
{
	FILE *in = fopen(fileName, "r");
	if (!in) {
		printf(
				"Child A: Error in opening input file...exiting with error code -1\n");
		return -1;
	}

	struct stat st;
	fstat(fileno(in), &st);
	char *fileContent = (char*) malloc(((int) st.st_size + 1) * sizeof(char));
	fileContent[0] = '\0';
	while (!feof(in)) {
		char line[100];
		if (fgets(line, 100, in) != NULL) {
			strncat(fileContent, line, strlen(line));
		}
	}
	fclose(in);

	char *command = NULL;
	int threadCount = 0;
	char *fileCopy = (char*) malloc((strlen(fileContent) + 1) * sizeof(char));
	strcpy(fileCopy, fileContent);
	command = strtok(fileCopy, "\r\n");
	while (command != NULL) {
		threadCount++;
		command = strtok(NULL, "\r\n");
	}
	*threads = (Thread*) malloc(sizeof(Thread) * threadCount);

	char *lines[threadCount];
	command = NULL;
	int i = 0;
	command = strtok(fileContent, "\r\n");
	while (command != NULL) {
		lines[i] = malloc(sizeof(command) * sizeof(char));
		strcpy(lines[i], command);
		i++;
		command = strtok(NULL, "\r\n");
	}

	for (int k = 0; k < threadCount; k++) {
		char *token = NULL;
		int j = 0;
		token = strtok(lines[k], ";");
		while (token != NULL) {
//if you have extended the Thread struct then here
//you can do initialization of those additional members
//or any other action on the Thread members
			(*threads)[k].state = 0;
			if (j == 0)
				strcpy((*threads)[k].tid, token);
			if (j == 1)
				(*threads)[k].startTime = atoi(token);
			j++;
			token = strtok(NULL, ";");
		}
	}
	return threadCount;
}


void logStart(char *tID) {
    printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char *tID) {
    printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

int threadsLeft(Thread *threads, int threadCount) {
    int remainingThreads = 0;
    for (int k = 0; k < threadCount; k++) {
        if (threads[k].state > -1)
            remainingThreads++;
    }
    return remainingThreads;
}

int threadToStart(Thread *threads, int threadCount) {
    int lowestStartTimeIndex = -1;
    unsigned int lowestStartTime = UINT_MAX;
    long currentTime = getCurrentTime();

    for (int k = 0; k < threadCount; k++) {
        if (threads[k].state == 0 && threads[k].startTime <= currentTime) {
            if (threads[k].startTime < lowestStartTime) {
                lowestStartTime = threads[k].startTime;
                lowestStartTimeIndex = k;
            }
        }
    }

    return lowestStartTimeIndex;
}

void* threadRun(void *t) {
    logStart(((Thread*) t)->tid);

    // Critical section starts here
    printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(),
        ((Thread*) t)->tid);
    // Critical section ends here

    logFinish(((Thread*) t)->tid);
    ((Thread*) t)->state = -1;
    pthread_exit(0);
}

void startClock() {
    programClock = time(NULL);
}

long getCurrentTime() {
    time_t now;
    now = time(NULL);
    return now - programClock;
}
