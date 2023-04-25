#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "defs.h"
#include "workers.h"
#include "generalFunctions.h"
#include "stats.h"



void execute_job(s_worker_job* worker_job, s_thread_data* thread_data)
{   
    useconds_t usec = 0;
    struct timeval stop_sleep, start_sleep;

    for(int i = 0; i < worker_job->repeat; i++)
    {
        for(int j = 0; worker_job->command_list[j].cmd[0] != 0; j++)
        {
            if (strcmp(worker_job->command_list[j].cmd, "msleep") == 0)
            {
                // sleep for arg milliseconds
                usec = (useconds_t)((worker_job->command_list[j].arg)*1000);
                gettimeofday(&start_sleep, NULL);
                usleep(usec);
                gettimeofday(&stop_sleep, NULL);

                long long int sleep_time = (long long int)(((double)((stop_sleep.tv_usec - start_sleep.tv_usec))/1000)+((stop_sleep.tv_sec - start_sleep.tv_sec) * 1000));
            }
            else if (strcmp(worker_job->command_list[j].cmd, "increment") == 0)
            {
                increment_counter(worker_job->command_list[j].arg);
            }
            else if (strcmp(worker_job->command_list[j].cmd, "decrement") == 0)
            {
                decrement_counter(worker_job->command_list[j].arg);
            }
            else
            {
                printf("ERROR: unknown command: %s, exiting\n", worker_job->command_list[j].cmd);
                exit(1);
            }
        }
    }
}                 


void* worker_job(void* data)
{
    long long int turnaround_time = 0;
    struct timeval thread_finish_time, dispatcher_read_time;
    extern pthread_mutex_t mutexQueue;
    extern pthread_mutex_t mutexStats;
    extern pthread_cond_t  cond;
    extern pthread_mutex_t mutexLiveThreads;
    extern s_node*         worker_queue_head;
    extern int             is_dispatcher_done;
    extern int             threads_running;
    char log_file_name[20];
    s_worker_job* worker_job = (s_worker_job*)malloc(sizeof(s_worker_job));
    memset(worker_job->command_list, 0, sizeof(worker_job->command_list));
    memset(worker_job->original_line, 0, sizeof(worker_job->original_line));

    while(1)
    {
        // critical section
        pthread_mutex_lock(&mutexQueue);
        while((worker_queue_head == NULL) && (is_dispatcher_done==0))
        {
            pthread_cond_wait(&cond, &mutexQueue);

            // if dispatcher is done, and work_queue is empty, exit
            if (is_dispatcher_done)
            {
                pthread_mutex_unlock(&mutexQueue);
                pthread_exit(NULL);
            }
        }
        pthread_mutex_unlock(&mutexQueue); // end of critical section
        


        // critical section
        pthread_mutex_lock(&mutexQueue);

        // copy the data from the queue to the worker_job
        if (worker_queue_head == NULL)
        {
            pthread_mutex_unlock(&mutexQueue);
            pthread_exit(NULL);
        }


        pthread_mutex_lock(&mutexLiveThreads);
        threads_running++;
        pthread_mutex_unlock(&mutexLiveThreads);

        worker_job->repeat = worker_queue_head->worker_job->repeat;
        int i=0;
        while(worker_queue_head->worker_job->command_list[i].cmd[0] != 0)
        {
            strcpy(worker_job->command_list[i].cmd, worker_queue_head->worker_job->command_list[i].cmd);
            worker_job->command_list[i].arg = worker_queue_head->worker_job->command_list[i].arg;
            i++;
        }
        strcpy(worker_job->original_line, worker_queue_head->worker_job->original_line);
        worker_job->line_read_time= worker_queue_head->worker_job->line_read_time;
        pop_worker_queue(&worker_queue_head);

        pthread_mutex_unlock(&mutexQueue);
        // end of critical section
        
        dispatcher_read_time = worker_job->line_read_time;
        sprintf(log_file_name, "thread%02d.txt", ((s_thread_data*)data)->thread_id);
        if (((s_thread_data*)data)->is_log_enabled)
        {
            WORKER_START_LOG(log_file_name, worker_job->original_line);
        }
        execute_job(worker_job, ((s_thread_data*)data));

        if (((s_thread_data*)data)->is_log_enabled)
        {
            WORKER_END_LOG(log_file_name, worker_job->original_line);
        }

        gettimeofday(&thread_finish_time, NULL);
        turnaround_time = (long long int)(((double)((thread_finish_time.tv_usec - dispatcher_read_time.tv_usec))/1000)+((thread_finish_time.tv_sec - dispatcher_read_time.tv_sec) * 1000)); // time in ms
 
        // critical section
        pthread_mutex_lock(&mutexStats);
        update_job_stats(turnaround_time);
        pthread_mutex_unlock(&mutexStats);

        pthread_mutex_lock(&mutexLiveThreads);
        threads_running--;
        pthread_mutex_unlock(&mutexLiveThreads);
    }
    free(worker_job);
}
   

s_node* create_worker_queue_node() {
    s_node* node = (s_node*)malloc(sizeof(s_node));
    if (!node) { 
        printf("allocation error\n");
        exit(1);
    }
    node->worker_job=NULL;
    node->next=NULL;
    return node;
}

s_worker_job* init_worker_job(char** parsed_cmd, char* original_line, struct timeval line_read_time)
{
    int i=0;
    int j=0;
    s_worker_job* worker_job = (s_worker_job*)malloc(sizeof(s_worker_job));

    memset(worker_job->command_list, 0, sizeof(worker_job->command_list));
    memset(worker_job->original_line, 0, sizeof(worker_job->original_line));

    if (!worker_job) { 
        printf("allocation error\n");
        exit(1);
    }

    if (!strcmp(parsed_cmd[1],"repeat")){
        int repeat_int=atoi(parsed_cmd[2]);
        worker_job->repeat=repeat_int;
        i=3;
    }
    else{
        worker_job->repeat=1;
        i=1;
    }

    for(i; parsed_cmd[i]!=NULL; i+=2){
        strcpy(worker_job->command_list[j].cmd, parsed_cmd[i]);
        worker_job->command_list[j].arg =atoi(parsed_cmd[i+1]);
        j++;
    }

    strcpy(worker_job->original_line, original_line);
    worker_job->line_read_time=line_read_time;

    return worker_job;
}

void push_worker_queue(s_node** head, s_worker_job *worker_job)
{
    s_node* new_node = create_worker_queue_node();

    if (*head==NULL) // list is empty
    {
        *head = new_node;
        new_node->worker_job = worker_job;
    }
    
    else
    {
        s_node* last = *head;
        new_node->worker_job = worker_job;

        while(last->next!=NULL)
        {
            last = last->next;
        }

        last->next = new_node;
    }
}

void pop_worker_queue(s_node** head)
{
    if (head == NULL)
    {
        printf("queue is already empty.\n");
        exit(1);
    }

    s_node* temp = *head;
    *head = (*head)->next;
    free(temp);
}

void free_worker_queue(s_node* head)
{
    s_node* temp;
    while(head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
 }

 void increment_counter(int counter_id)
 {
    FILE* counter_file = NULL;
    extern pthread_mutex_t mutexCounters;
    int counter_value = 0;
    char file_name[COUNTER_FILE_NAME_LEN] = {0};
    sprintf(file_name, "count%02d.txt", counter_id);

    pthread_mutex_lock(&(mutexCounters)+counter_id);
    counter_file = fopen(file_name, "r");
    if (counter_file == NULL)
    {
        printf("ERROR: failed to open file %s\n", file_name);
        exit(1);
    }
    else
    {
        fscanf(counter_file, "%d", &counter_value);
        fclose(counter_file);
        counter_file = fopen(file_name, "w");
        counter_value++;
        fprintf(counter_file, "%d", counter_value);
        fclose(counter_file);
    }
    pthread_mutex_unlock(&(mutexCounters)+counter_id);
 }


 void decrement_counter(int counter_id)
 {
    FILE* counter_file = NULL;
    extern pthread_mutex_t mutexCounters;
    int counter_down = 0;
    char file_name[COUNTER_FILE_NAME_LEN] = {0};
    sprintf(file_name, "count%02d.txt", counter_id);

    pthread_mutex_lock(&(mutexCounters)+counter_id);
    counter_file = fopen(file_name, "r");
    if (counter_file == NULL)
    {
        printf("ERROR: failed to open file %s\n", file_name);
        exit(1);
    }
    else
    {
        fscanf(counter_file, "%d", &counter_down);
        fclose(counter_file);
        counter_file = fopen(file_name, "w");
        counter_down-=1;
        fprintf(counter_file, "%d", counter_down);
        fclose(counter_file);
    }
    pthread_mutex_unlock(&(mutexCounters)+counter_id);
 }

