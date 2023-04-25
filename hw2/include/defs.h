#ifndef DEFS_H
#define DEFS_H

#define MAX_NUM_OF_WORKERS    4096
#define MAX_NUM_OF_COUNTERS   100
#define MAX_CMD_LEN           1024
#define MAX_WORD_LEN          32
#define LOG_FILE_NAME_LEN     32
#define COUNTER_FILE_NAME_LEN 12


enum log_type{
    LOG_TYPE_DISPATCHER,
    LOG_TYPE_WORKER,
    LOG_TYPE_LAST
};

#endif