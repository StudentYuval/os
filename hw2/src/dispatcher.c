#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "generalFunctions.h"
#include "dispatcher.h"
#include "workers.h"
#include "defs.h"

extern pthread_mutex_t mutexLiveThreads;
extern int threads_running;

void init_dispatcher(int argc,char ** argv, s_input_args* input_args, pthread_t* workers, s_thread_data* thread_data)
{
    parse_args(argc, argv, input_args);    
    // create num_threads threads and log files
    for (int i=0; i<(input_args->num_of_threads); i++)
    {
        thread_data[i].thread_id=i;
        thread_data[i].is_log_enabled=input_args->is_log_enabled;

        pthread_create(workers+i, NULL, (void*)worker_job, (void*)(thread_data+i)); // check NULL
        if (input_args->is_log_enabled)
        {
            char log_file_name[LOG_FILE_NAME_LEN];
            sprintf(log_file_name, "thread%02d.txt", i);
            create_new_file(log_file_name);
        }
    }

    // create num_counters counter files
    for (int i=0; i<(input_args->num_of_counters); i++)
    {
        char counter_file_name[COUNTER_FILE_NAME_LEN];
        sprintf(counter_file_name, "count%02d.txt", i);
        create_new_file(counter_file_name);

        FILE* counter_file = fopen(counter_file_name, "w");
        if (counter_file == NULL) {
            printf("Error opening file!\n");
            exit(1);
        }
        fprintf(counter_file, "0");
        fclose(counter_file);
    }

    // create dispatcher log file
    if (input_args->is_log_enabled)
    {
        create_new_file("dispatcher.txt");
    }
}


void dispatcher_job(s_input_args* input_args ,char** parsed_cmd, char* original_line, pthread_t* workers)
{
    useconds_t usec = 0;

    if(input_args->is_log_enabled)
    {
        DISPATCHER_LOG(original_line);
    }
    
    if (!strcmp(parsed_cmd[1],"msleep"))
    {
        // sleep for arg milliseconds
        usec = (useconds_t)(atoi(parsed_cmd[2])*1000);
        usleep(usec);
    }
    else if (!strcmp(parsed_cmd[1],"wait"))
    {       

      // sleep as long as there are threads execution a job
        while(1)
        {
            pthread_mutex_lock(&mutexLiveThreads);
            if (threads_running == 0)
            {
                pthread_mutex_unlock(&mutexLiveThreads);
                break;
            }
            pthread_mutex_unlock(&mutexLiveThreads);
        }
    }

    else {
        printf("illigal dispatcher command");
        exit(1);
    }

}

void dispatcher(s_input_args* input_args, FILE* cmd_file, pthread_t* workers)
{
    extern s_node* worker_queue_head;
    extern pthread_mutex_t mutexQueue;
    extern pthread_cond_t cond;
    extern int is_dispatcher_done;
    struct timeval dispatcher_read_time;
    
    while(1)
    {
        char* line;
        line = read_line(cmd_file);

        if (line == NULL)
        {
            break;
        }
        gettimeofday(&dispatcher_read_time, NULL);
        //adding '\n' to the end of line in case its missing
        int len = strlen(line);
        if (line[len-1]!='\n')
        {
            line[len] ='\n';
            line[len+1] ='\0';
        }

        // save a copy of the original line for logging
        char original_line[MAX_CMD_LEN];
        strcpy(original_line, line);

        //parse line     
        char** cmd = parse_cmd_line(line);


        if (!strcmp(cmd[0], "dispatcher")){
            dispatcher_job(input_args, cmd, original_line, workers);
        }

        if (!strcmp(cmd[0], "worker")){
        //init and add cmd to workers queue
            s_worker_job* worker_job = init_worker_job(cmd, original_line,dispatcher_read_time);

            /* push the job to the queue. protect the queue with mutex, and signal workers they can take a job while the queue is not empty */
            pthread_mutex_lock(&mutexQueue);
            push_worker_queue(&worker_queue_head, worker_job);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutexQueue);
        }

        free(cmd);
    }

    // if there are no more commands, and queue is empty - make all workers to wake up and finish

    is_dispatcher_done = 1;
    
    // wake up all workers
    pthread_mutex_lock(&mutexQueue);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutexQueue);
}