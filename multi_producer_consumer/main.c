//
//  main.c
//  multi_producer_consumer
//
//  Created by Ge Wang on 6/2/14.
//  Copyright (c) 2014 Ge Wang. All rights reserved.
//

#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* The mutex lock */
pthread_mutex_t mutex;

/* the semaphores */
sem_t full, empty;

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

/* buffer counter */
int counter;
/* buffer tracker */
int in, out;

pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes

void initializeBuffer();
void *producer(void *param);
void *consumer(void *param);
int insert_item(buffer_item item);
int remove_item(buffer_item *item);

int main(int argc, const char * argv[])
{
    int sleep_sec, producers, consumers;
    /* Loop counter */
    int i;
    
    /* Verify the correct number of arguments were passed in */
    if(argc != 4) {
        fprintf(stderr, "USAGE:./main.out <INT> <INT> <INT>\n");
    }
    
    sleep_sec = atoi(argv[1]); /* Time in seconds for main to sleep */
    producers= atoi(argv[2]); /* Number of producer threads */
    consumers = atoi(argv[3]); /* Number of consumer threads */
    
    /* Initialize the app */
    initializeBuffer();
    printf("Main thread beginning\n");
    /* Create the producer threads */
    for(i = 0; i < producers; i++) {
        /* Create the thread */
        pthread_create(&tid,&attr,producer,NULL);
        printf("Creating producer thread with id %ul\n", tid);
    }
    
    /* Create the consumer threads */
    for(i = 0; i < consumers; i++) {
        /* Create the thread */
        pthread_create(&tid,&attr,consumer,NULL);
        printf("Creating consumer thread with id %ul\n", tid);
    }
    
    /* Sleep for the specified amount of time in milliseconds */
    printf("Main thread sleeping for %d seconds\n", sleep_sec);
    sleep(sleep_sec);
    
    sem_destroy(&full);
    sem_destroy(&empty);
    /* Exit the program */
    printf("Main thread exiting\n");
    exit(0);
}

void initializeBuffer() {
    
    /* Create the mutex lock */
    pthread_mutex_init(&mutex, NULL);
    
    /* Create the full semaphore and initialize to 0 */
    sem_init(&full, 0, 0);
    
    /* Create the empty semaphore and initialize to BUFFER_SIZE */
    sem_init(&empty, 0, BUFFER_SIZE);
    
    /* Get the default attributes */
    pthread_attr_init(&attr);
    
    /* init buffer */
    counter = 0;
    in = 1;
    out = 1;
}

/* Producer Thread */
void *producer(void *param) {
    buffer_item item;

    while(1) {
        /* sleep for a random period of time */
        int psleep = rand() % 3 + 1;
        printf("Producer thread %ul sleeping for %d seconds\n", pthread_self(), psleep);
        sleep(psleep);
        
        /* generate a random number */
        item = rand();
        /* acquire the empty lock */
        sem_wait(&empty);
        /* acquire the mutex lock */
        pthread_mutex_lock(&mutex);
        
        if(insert_item(item)) {
            fprintf(stderr, "Producer report error condition\n");
        }
        else {
            printf("Producer thread %ul inserted value %d\n", pthread_self(), item);
        }
        /* release the mutex lock */
        pthread_mutex_unlock(&mutex);
        /* signal full */
        sem_post(&full);
    }
}

/* Consumer Thread */
void *consumer(void *param) {
    buffer_item item;
    
    while(1) {
        /* sleep for a random period of time */
        int csleep = rand() % 3 + 1;
        printf("Consumer thread %ul sleeping for %d seconds\n", pthread_self(), csleep);
        sleep(csleep);
        /* aquire the full lock */
        sem_wait(&full);
        /* aquire the mutex lock */
        pthread_mutex_lock(&mutex);
        
        if(remove_item(&item)) {
            fprintf(stderr, "Consumer report error condition\n");
        }
        else {
            printf("Consumer thread %ul removed value %d\n", pthread_self(), item);
        }
        /* release the mutex lock */
        pthread_mutex_unlock(&mutex);
        /* signal empty */
        sem_post(&empty);
    }
}

/* Add an item to the buffer */
int insert_item(buffer_item item) {

    /* When the buffer is not full add the item
     and increment the counter*/
    if(counter < BUFFER_SIZE) {
        buffer[in - 1] = item;
        printf("Insert_item inserted item %d at position %d\n", item, in);
        /* Print out each entry of the buffer */
        for (int i = 0; i<BUFFER_SIZE; i++){
            if (buffer[i] == 0){
                printf("[empty]");
            } else {
                printf("[%d]",buffer[i]);
            }
        }
        counter++;
        /* change in to point to next empty slot in buffer */
        if (in < 4 || in == 5){
            in = (in + 1) % BUFFER_SIZE;
        } else if (in == 4){
            in = 5;
        }
        printf("in = %d, out = %d\n", in, out);
        return 0;
    }
    else { /* Error the buffer is full */
        return -1;
    }
}

/* Remove an item from the buffer */
int remove_item(buffer_item *item) {
    
    /* When the buffer is not empty remove the item
     and decrement the counter */
    if(counter > 0) {
        *item = buffer[out-1];
        buffer[out - 1] = 0;
        printf("Remove_item removed item %d at position %d\n", *item, out);
        /* Print out each entry of the buffer */
        for (int i = 0; i<BUFFER_SIZE; i++){
            if (buffer[i] == 0){
                printf("[empty]");
            } else {
                printf("[%d]",buffer[i]);
            }
        }
        counter--;
        /* change out to pointing to next available slot */
        if (out < 4 || out == 5){
            out = (out + 1) % BUFFER_SIZE;
        } else if (out == 4){
            out = 5;
        }
        printf("in = %d, out = %d\n", in, out);
        return 0;
    }
    else { /* Error buffer empty */
        return -1;
    }
}
