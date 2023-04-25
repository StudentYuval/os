#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "defs.h"
#include "generalFunctions.h"
#include "workers.h"
#include "dispatcher.h"
#include "stats.h"

//TOOD: add statistics module


//Globals
pthread_mutex_t mutexQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCounters[MAX_NUM_OF_COUNTERS] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutexLiveThreads = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexStats = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
s_node*         worker_queue_head = NULL;
struct timeval start_time;
int             is_dispatcher_done = 0;
int             threads_running = 0;

int main(int argc, char** argv)
{
    gettimeofday(&start_time, NULL);
    s_input_args   input_args = {};
    pthread_t      workers[MAX_NUM_OF_WORKERS]; //list of threads
    s_thread_data  thread_data[MAX_NUM_OF_WORKERS] = {0};
    FILE*          cmd_file = NULL;

    // dispatcher init
    init_dispatcher(argc, argv, &input_args, workers, thread_data);
   
    // open cmd_file
    if((cmd_file = fopen(input_args.file_name, "r")) == NULL)
    {
        printf("Error opening file %s, exiting...\n", input_args.file_name);
        exit(1);
    }

    // dispatcher job
    dispatcher(&input_args, cmd_file, workers);

    // close cmd_file
    fclose(cmd_file);


    for (int i=0; i<(input_args.num_of_threads); i++)
    {
        pthread_join(workers[i], NULL); // check NULL
    }

    pthread_mutex_destroy(&mutexQueue);
    pthread_mutex_destroy(&mutexLiveThreads);
    pthread_mutex_destroy(&mutexStats);
    for (int i=0; i<MAX_NUM_OF_COUNTERS; i++)
    {
        pthread_mutex_destroy(&mutexCounters[i]);
    }

    pthread_cond_destroy(&cond);
    free_worker_queue(worker_queue_head);

    // calculate total run time
    dump_stats();
    return 0;
}