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
#include <unistd.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024


// structs:
typedef struct request {
   int fd;
   char request[BUFF_SIZE];
} request_t;

typedef struct request_queue {
	  int len, front, back;
    request_t requests[MAX_queue_len];
} request_queue_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

typedef struct cache_queue {
	  int len, front, back;
    cache_entry_t caches[MAX_CE];
} cache_queue_t;

// globals:
request_queue_t * request_queue;
cache_queue_t * cache_queue;
int requests_qlen;
int ce_len;
int reqNum;
pthread_mutex_t dispatch_lock;
pthread_mutex_t worker_lock;
pthread_cond_t dispatch_cond_lock;
pthread_cond_t worker_cond_lock;


/**********************************************************************************/

/* ******************************** Request Queue Code ****************************/

// Function to add the request and its filename into the request queue
int addIntoRequestQueue(int fd, char* filename){
  // Determines whether adding a new request will exceed the maximum length.
  // Otherwise, adds the new request to the front and update the index parameters.
  if (request_queue->len >= requests_qlen){
	  return -1;
  }

  // Put the request into the queue by filling the request fields with the parameters.
  request_queue->requests[request_queue->back].fd = fd;
  strcpy(request_queue->requests[request_queue->back].request,filename);

  // request_queue is a queue array implementation. Update back index accordingly.
  if (request_queue->back = requests_qlen) {
	request_queue->back = 0;
  }
  else {
	request_queue->back++;
  }
  request_queue->len++;
  return 0;
}

// Function to take a request from the request queue
int takeFromRequestQueue(request_t * request){
  // Takes requests in FIFO order through index front and back pointer manipulation.
  // If there are no request in the array, return -1; else, fill in the request and return 0;
  if (request_queue->len <= 0){
	  return -1;
  }
  // Retrieve request from queue and fill in the paramter.
  request_t retrieved_request = request_queue->requests[request_queue->front];
  request->fd = retrieved_request.fd;
  strcpy(request->request, retrieved_request.request);

  // Remove the request from the request queue.
  retrieved_request.fd = -1;
  memset(retrieved_request.request, 0, sizeof(retrieved_request.request));

  // request_queue is a queue array implementation. Update front index accordingly.
  if (request_queue->front = requests_qlen) {
	request_queue->front = 0;
  }
  else {
	request_queue->front++;
  }
  request_queue->len--;
  return 0;
}

// Function to initialize the request queue
int initRequestQueue(){
  // Allocating memory and initializing the request struct.
  if( (request_queue = (request_queue_t *) malloc( sizeof(request_queue_t) )) == NULL){
    perror("failed to allocate memory for request queue");
    return -1;
  }
  request_queue->len = 0;
  request_queue->front = 0;
  request_queue->back = 0;
  return 0;
}

// clear the memory allocated to the request queue
void deleteRequestQueue(){
  // De-allocate/free the request queue memory
  free(request_queue);
}


/********************************************************************************/

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
  for (int i=0; i<MAX_CE; i++){
	if( strcmp(cache_queue[i].request, request) == 0 ){
      return i;
    }
  }
  return -1;
}


// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memeory when adding or replacing cache entries
  if(cache_queue->len > ce_len)
  {
    cache_queue->index = 0;
  }
  cache_queue->caches[cache_queue->index].len = memory_size;
  strcpy(cache_queue->caches[cache_queue->index].request, mybuf);
  strcpy(cache_queue->caches[cache_queue->index].content, memory);
  cache_queue->len++;
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
  free(cache_queue);
}

// Function to initialize the cache
int initCache(){
  // Allocating memory and initializing the cache array.
  if( (cache_queue = (cache_queue_t *) malloc( sizeof(cache_queue_t) )) == NULL){
    perror("failed to allocate memory for cache queue");
    return -1;
  }
  cache_queue->len = 0;
  cache_queue->index = 0;
  return 0;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(char * request) {
  // Open and read the contents of file given the request
  return read(request);
}


/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
   static char contentType[BUFF_SIZE]; //will return this
   const char ch = '.';
   char *str = strrchr(mybuf, ch); //get information after the .

   if( (strcmp(str, "htm") == 0) || (strcmp(str, "html") == 0)){
     strcpy(contentType, "text/html");
     return contentType;
   }
   else if(strcmp(str, "jpg") == 0){
     strcpy(contentType, "image/jpeg");
     return contentType;
   }
   else if (strcmp(str, "gif") == 0){
     strcpy(contentType, "image/gif");
     return contentType;
   }
   else{
     strcpy(contentType, "text/plain");
     return contentType;
   }
}

// This function returns the current time in microseconds
long getCurrentTimeInMicro() {
  struct timeval curr_time;
  gettimeofday(&curr_time, NULL);
  return curr_time.tv_sec * 1000000 + curr_time.tv_usec;
}

/**********************************************************************************/


// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {

  while (1) {

    // Accept client connection
	int fd = accept_connection();
	if(fd < 0){
        fprintf(stderr, "failed to accept client connection\n");
        continue;
    }

    // Get request from the client
	char filename[BUFF_SIZE];
	if (get_request(fd, filename) != 0){
		fprintf(stderr, "get_request() failed\n");
        close(fd);
        continue;
	}

    // Add the request into the queue

	// ------------- LOCK -------------
	pthread_mutex_lock(&dispatch_lock);
	// --------------------------------
  while (addIntoRequestQueue(fd, filename) != 0){
		pthread_cond_wait(&dispatch_cond_lock, &dispatch_lock);
	}
	pthread_cond_signal(&worker_cond_lock);
	// ------------- UNLOCK -------------
	pthread_mutex_unlock(&dispatch_lock);
	// ----------------------------------

   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

  int threadId = pthread_self();
  int fd;
    char *content_type;
    char buf[BUFF_SIZE];
    int numbytes;

   while (1) {

    // Start recording time
	long start = getCurrentTimeInMicro();

    // Get the request from the queue
    request_t * request = malloc(sizeof(request_t));
     while(request_queue->len == 0){ // when there no request in queue
       //wait
     }
    int result = takeFromRequestQueue(request);
     	//if there request on queue, sent it back to client
     if(result < 0 ){
       perror("Fail to get request from queue");
       continue;
     }

    // Get the data from the disk or the cache
     	//look up request from cache, if HIT, it will get result from cache and return to the user
    int cacheIndex;
    char cache_status[BUFF_SIZE];
    memset(cache_status, 0, sizeof(cache_status));
		if (cacheIndex = getCacheIndex(request) < 0  ) {
      //request is not in cache
      // Now, we read from disk.
      cacheIndex = readFromDisk(request->request);
      strcpy(cache_status, "MISS");
    }
    else{
      strcpy(cache_status, "HIT");
    }
     if (cacheIndex < 0){
       perror("Fail to read from Disk??");
       continue;
     }

    fd = request->fd;
    content_type =  getContentType(request->request);
    if (result = return_result(fd, content_type, buf, sizeof(request_t))< 0 ){
      perror("Fail to return result");
    }

    // Stop recording the time
	long end = getCurrentTimeInMicro();
	long total_time = end - start;

    // Log the request into the file and terminal
     FILE *fp;
     fp = fopen("webserver_log", "w+");
     if(fp == NULL) {
       perror("Error opening webserver_log file");
     }
     char str[BUFF_SIZE];
     sprintf(str, "\n[%d][%d][%d][%s][%d][%d][%s]", threadId,reqNum, fd, request->request,bytes, total_time, cache_status);
   //  fputs(str, fp);
     fwrite(str,strlen(str)+1,sizeof(str), fp);
     printf("%s \n", str);
   //  fwrite(threadId, reqNum, fd, request->request, bytes, total_time, cache_status, fp);
     fclose(fp);


    // return the result
     return result;

  }
  return NULL;
}

/**********************************************************************************/

// Function to check errors on input arguments
int check_parameters(int parameters[]) {
   // parameter = [port, 1, num_dispatcher, num_workers, dynamic_flag, qlen, cache_entries]
   char * parameter_type[] = {"port", "path", "num_dispatcher", "num_workers", "dynamic_flag", "qlen", "cache_entries"};

   // Check if atoi() succeeded.
   for (int i = 0; i < 7; i++){
     if (parameters[i]==0 && strcmp(parameter_type[i+1],"0")!=0){
       printf("invalid parameter: %s\n", parameter_type[i]);
       return -1;
     }
   }

   // Check bounds.
   if (parameters[0]<1025 || parameters[0]>65535){
     printf("port number is out of bound; must be bounded between 1025 and 65535\n");
     return -1;
   }
   if ((parameters[2]+parameters[3])>MAX_THREADS || (parameters[2]+parameters[3])<0){
     printf("insufficient number of threads to process the input number of dispatchers and/or workers\n");
     return -1;
   }
   if (parameters[4]<0 || parameters[4]>1){
     printf("invalid dynamic_flag; must be 0 or 1\n");
     return -1;
   }
   if (parameters[5]>MAX_queue_len){
     printf("qlen exceeds the maximum length of the request queue allowed: %d\n", MAX_queue_len);
     return -1;
   }
   if (parameters[6]>MAX_CE){
     printf("cache_entries exceeds the maximum number of cache entries allowed: %d\n", MAX_CE);
     return -1;
   }

   return 0;
}
/***********************************************************************************/
/*logging function*/
void logFileAndTerminal(){

}

/**********************************************************************************/

int main(int argc, char **argv) {

  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args
  char* path = argv[2];
  int port, num_dispatcher, num_workers, dynamic_flag, qlen, cache_entries;

  port = atoi(argv[1]);
  num_dispatcher = atoi(argv[3]);
  num_workers = atoi(argv[4]);
  dynamic_flag = atoi(argv[5]);
  qlen = atoi(argv[6]);
  cache_entries = atoi(argv[7]);

  // Perform error checks on the input arguments
  int parameters[] = {port, num_dispatcher, num_workers, dynamic_flag, qlen, cache_entries};
  if (check_parameters(parameters) < 0){
    // relevant print error statements are achieved in check_parameters()
    return -1;
  }

  // Change the current working directory to server root directory
  if (chdir(path) != 0){
    printf("path name does not exist or is invalid\n");
    return -1;
  }

  // Start the server and initialize cache
  // Also initialize thread locks.
  if ( (pthread_mutex_init(&dispatch_lock, NULL) != 0) || (pthread_mutex_init(&worker_lock, NULL) != 0)){
    printf("mutex init failed\n");
    return -1;
  }

  if ( (pthread_cond_init(&dispatch_cond_lock, NULL) != 0) || (pthread_cond_init(&worker_cond_lock, NULL) != 0)){
    printf("mutex cond int failed\n");
    return -1;
  }
  init(port);
  if (initRequestQueue()!=0){
    return -1;
  }
  if (initCache()!=0){
    deleteRequestQueue();
    return -1;
  }

  // Create dispatcher and worker threads
  pthread_t dispatch_threads_pools[num_dispatcher];
  pthread_t worker_threads_pools[num_workers];
  pthread_t dispatch_threads, worker_threads;

  int dis_num = num_dispatcher;
  while (dis_num > 0) {
    /* Create dispatcher thread */
    dis_num--;
  	if (pthread_create(&dispatch_threads,NULL,dispatch,NULL) != 0) {
  		perror("couldn't create dispatcher thread");
  	}
  }

  int work_num = num_workers;
  while (work_num > 0) {
    /* Create worker thread */
    work_num--;
  	if (pthread_create(&worker_threads,NULL,worker,NULL) != 0) {
  		perror("couldn't create worker thread");
  	}
  }

  // Clean up
  pthread_mutex_destroy(&dispatch_lock);
  pthread_cond_destroy(&dispatch_cond_lock);
  pthread_mutex_destroy(&worker_lock);
  pthread_cond_destroy(&worker_cond_lock);
  deleteRequestQueue();
  deleteCache();
  return 0;
}
