#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // Include for sleep()

#define MAX_RESOURCES 5
#define NUM_THREADS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int avail_resources = MAX_RESOURCES;

int decrease_count(int count){
    pthread_mutex_lock(&mutex);

    while(avail_resources < count){
        pthread_cond_wait(&cond, &mutex);
    }
    // Decrease resources and return
    avail_resources -= count;
    pthread_mutex_unlock(&mutex);
    return 0;
}

int increase_count(int count){
    pthread_mutex_lock(&mutex);
    avail_resources += count;
    // Signal cond, unlock and return
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void* thread_function(void* arg){
    int thread_num = *(int*)arg;
    decrease_count(1); // Try to acquire 1 resource
    printf("The thread %d has acquired, 1 resources and %d more resources are available.\n", thread_num, MAX_RESOURCES - avail_resources);
    // Simulate some work by sleeping
    sleep(1);
    increase_count(1); // Release the resource
    printf("The thread %d has released, 1 resources and %d resources are now available.\n", thread_num, avail_resources);
    return NULL;
}

int main(){
    pthread_t threads[NUM_THREADS];
    int thread_nums[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++){
        thread_nums[i] = i;
        if(pthread_create(&threads[i], NULL, thread_function, &thread_nums[i])){
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("All threads have finished execution. Available resources: %d\n", avail_resources);

    return 0;
}
