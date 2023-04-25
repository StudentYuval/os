#ifndef WORKERS_H
#define WORKERS_H
#include <sys/time.h>
#include "defs.h"

struct s_cmd{
    char  cmd[MAX_WORD_LEN];
    int   arg;
};

struct s_input_args{               // represents the input arguments
    char* file_name;
    int   num_of_threads;
    int   num_of_counters;
    int   is_log_enabled;
};

struct s_worker_job{                // represents a single line of the input file
    int            repeat;
    struct s_cmd   command_list[MAX_CMD_LEN];
    char           original_line[MAX_CMD_LEN];
    struct timeval line_read_time;
};

struct s_node{                       //worker queue linked list
    struct s_worker_job* worker_job;
    struct s_node *next;
};



struct s_thread_data{             // represents the data that is passed to each thread
    int thread_id;
    int is_log_enabled;
};


// typedefs
typedef struct s_cmd s_cmd;
typedef struct s_input_args s_input_args;
typedef struct s_worker_job s_worker_job;
typedef struct s_node s_node;
typedef struct s_thread_data s_thread_data;


// functions declarations
s_node*       create_worker_queue_node();
s_worker_job* init_worker_job(char** parsed_cmd, char* original_line, struct timeval line_read_time);
void*         worker_job(void *data);
void          push_worker_queue(s_node** head, s_worker_job *worker_job);
void          pop_worker_queue(s_node** head);
void          free_worker_queue(s_node* head);
void          increment_counter(int counter_id);
void          decrement_counter(int counter_id);

#endif