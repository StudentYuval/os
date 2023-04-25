#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

s_stats g_stats;

void update_total_time(long long int total_time)
{
    g_stats.total_time = total_time;
}

// dump stats into file called stats.txt
void dump_stats()
{
    extern struct timeval start_time;
    FILE* stats_file = NULL;
    if((stats_file = fopen("stats.txt", "w")) == NULL)
    {
        printf("Error opening file stats.txt, exiting...\n");
        exit(1);
    }

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long long int total_time = (long long int)(((double)((current_time.tv_usec - start_time.tv_usec))/1000)+((current_time.tv_sec - start_time.tv_sec) * 1000)); // time in ms
    update_total_time(total_time);

    fprintf(stats_file, "total running time: %lld milliseconds\n", g_stats.total_time);
    fprintf(stats_file, "sum of jobs turnaround time: %lld milliseconds\n", g_stats.sum_turnaround_time);
    fprintf(stats_file, "min of jobs turnaround time: %lld milliseconds\n", g_stats.min_turnaround_time);
    fprintf(stats_file, "average of jobs turnaround time: %lld milliseconds\n", g_stats.avg_turnaround_time);
    fprintf(stats_file, "max of jobs turnaround time: %lld milliseconds\n", g_stats.max_turnaround_time);

    fclose(stats_file);
}

void update_job_stats(long long int turnaround_time)
{
    g_stats.num_of_jobs++;
    g_stats.sum_turnaround_time += turnaround_time;
    if (turnaround_time < g_stats.min_turnaround_time || g_stats.min_turnaround_time == 0)
    {
        g_stats.min_turnaround_time = turnaround_time;
    }
    if (turnaround_time > g_stats.max_turnaround_time)
    {
        g_stats.max_turnaround_time = turnaround_time;
    }
    g_stats.avg_turnaround_time = g_stats.sum_turnaround_time / g_stats.num_of_jobs;
}