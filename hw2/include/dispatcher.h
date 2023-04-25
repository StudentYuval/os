#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdio.h>
#include <stdlib.h>
#include "workers.h"


void dispatcher_job(s_input_args* input_args ,char** parsed_cmd, char* original_line, pthread_t* workers);
void dispatcher(s_input_args* input_args, FILE* cmd_file, pthread_t* workers);
void init_dispatcher(int argc,char ** argv, s_input_args* p_input_args, pthread_t* workers, s_thread_data* thread_data);

#endif