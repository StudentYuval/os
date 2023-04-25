#ifndef GENERAL_FUNCTIONS_H
#define GENERAL_FUNCTIONS_H

#include "defs.h"
#include "workers.h"

void parse_args(int argc, char* argv[], s_input_args* args);
void parse_cmd_file(s_input_args* args, char** cmd_list);
char **parse_cmd_line (char* cmd_line);
void create_new_file(char* file_name);
char* read_line(FILE* file);
void write_to_log(char* file_name, char* original_cmd_string, enum log_type type, int is_start);

#define DISPATCHER_LOG(original_cmd) write_to_log("dispatcher.txt", original_cmd,  LOG_TYPE_DISPATCHER, 0)
#define WORKER_START_LOG(file, original_cmd) write_to_log(file, original_cmd,  LOG_TYPE_WORKER, 1)
#define WORKER_END_LOG(file, original_cmd) write_to_log(file, original_cmd,  LOG_TYPE_WORKER, 0)

#endif