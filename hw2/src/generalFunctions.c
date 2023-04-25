#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "workers.h"
#include "generalFunctions.h"
#include <sys/time.h>

void parse_args(int argc, char* argv[], s_input_args* args) {
    
    if (argc != 5) {
        printf("Invalid input.\nUsage: hw2 <cmd_file> <num_threads> <num_counters> <trace_enabled>\n");
        return;
    }
    else{
        args->file_name = argv[1];
        args->num_of_threads = atoi(argv[2]);
        args->num_of_counters = atoi(argv[3]);
        args->is_log_enabled = atoi(argv[4]);
    }
}

// function to read a single line from txt file and return that line - and keep the file pointer in the right place
char* read_line(FILE* file){
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    if ((read = getline(&line, &len, file)) != -1)
    {
        return line;
    }
    else
    {
        return NULL;
    }
}

char **parse_cmd_line (char cmd_line[MAX_CMD_LEN]){
    int index = 0;
    char **tokens = malloc(MAX_CMD_LEN * sizeof(char*)); 
    char *token;

    if (!tokens) { 
        printf("allocation error\n");
        exit(1);
    }

    token = strtok(cmd_line, " _;\n\t");
    while (token != NULL) {
        tokens[index] = token;
        index++;

        token = strtok(NULL,  " _;\n\t");
    }
    return tokens;
}

void create_new_file(char* file_name) {
    FILE* file = fopen(file_name, "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fclose(file);
}

void write_to_log(char* file_name, char* original_cmd_string, enum log_type type, int is_start)
{
    FILE* file = fopen(file_name, "a");
    extern struct timeval start_time;
    struct timeval current_time;
    long long int passed_time;

    gettimeofday(&current_time, NULL);
    passed_time = (long long int)(((double)((current_time.tv_usec - start_time.tv_usec))/1000)+((current_time.tv_sec - start_time.tv_sec) * 1000)); // time in ms

    if (file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    switch(type)
    {
        case LOG_TYPE_DISPATCHER:
                        fprintf(file, "TIME %lld: read cmd line: %s",passed_time, original_cmd_string);
            break;

        case LOG_TYPE_WORKER:
            if (is_start)
                fprintf(file, "TIME %lld: START job %s",passed_time, original_cmd_string);
            else
                fprintf(file, "TIME %lld: END job %s",passed_time, original_cmd_string);
            break;

        default:
            break;
    }

    fclose(file);
}