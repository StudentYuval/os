#ifndef STATS_H
#define STATS_H

struct s_stats
{
    long long int total_time;
    long long int sum_turnaround_time;
    long long int min_turnaround_time;
    long long int max_turnaround_time;
    long long int avg_turnaround_time;
    unsigned int  num_of_jobs;
};

typedef struct s_stats s_stats;

void dump_stats();
void update_job_stats(long long int turnaround_time);

#endif