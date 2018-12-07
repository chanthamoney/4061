#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t worker_mutex;
//pthread_cond_t  count_condv;

/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGESSTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   int front;
   int rear;
   unsigned capacity;
   int size;
   void *request;
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int* array;
};

// function to create a queue of given capacity.
// It initializes size of queue as 0
struct request_queue* createQueue(unsigned capacity)
{
    struct request_queue* queue = (struct request_queue*) malloc(sizeof(struct request_queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // This is important, see the enqueue
    queue->request = (void*) malloc(queue->capacity * sizeof(request_t));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct request_queue* queue)
{  return (queue->size == queue->capacity);  }

// Queue is empty when size is 0
int isEmpty(struct request_queue* queue)
{  return (queue->size == 0); }

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct request_queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.
// It changes front and size
int dequeue(struct request_queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct request_queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}

// Function to get rear of queue
int rear(struct request_queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}
/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memeory when adding or replacing cache entries
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
  // Open and read the contents of file given the request
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
}

// This function returns the current time in milliseconds
int getCurrentTimeInMills() {
  struct timeval curr_time;
  gettimeofday(&curr_time, NULL);
  return curr_time.tv_usec;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {

  while (1) {

    // Accept client connection
    int fd = accept_connection();
    if(fd>0){

      // Get request from the client
      request_queue* temp = (request_queue*)arg;
      get_request(temp->fd, (char*)temp->request);
      // Add the request into the queue
      enqueue(arg);
    }

   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

   while (1) {

    // Start recording time

    // Get the request from the queue
    dequeue(arg);

    // Get the data from the disk or the cache

    // Stop recording the time

    // Log the request into the file and terminal

    // return the result
  }
  return NULL;
}

/**********************************************************************************/

int main(int argc, char **argv) {

  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args
  int port = atoi(argv[1]);
  char* path = argv[2];
  int num_dispatcher = atoi(argv[3]);
  int num_workers = atoi(argv[4]);
  int dynamic_flag = atoi(argv[5]);
  int qlen = atoi(argv[6]);
  int cache_entry = atoi(argv[7]);


  // Perform error checks on the input arguments
  init(port);
  pthread_t dispatch_threads_pools[num_dispatcher];
  pthread_t worker_threads_pools[num_workers];

  // Change the current working directory to server root directory

  // Start the server and initialize cache

  // Create dispatcher and worker threads
  int dis_num = num_dispatcher;
  int work_num = num_workers;
  pthread_t dispatch_threads, worker_threads;
  while (dis_num > 0) {
    /* Create dispatcher thread */
    dis_num--;
  	if (pthread_create(&dispatch_threads,NULL,dispatch,NULL) != 0) {
  		perror("couldn't create dispatcher thread");
  	}

  }

  while (work_num > 0) {
    /* Create worker thread */
    work_num--;
  	if (pthread_create(&worker_threads,NULL,worker,NULL) != 0) {
  		perror("couldn't create worker thread");
  	}

  }

  // Clean up
  return 0;
}
