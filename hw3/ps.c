/*iterate over all active processes in the
system, and print their information to screen, just like the ps command does in Linux.*/

#include "types.h"
#include "user.h"

#define STDOUT 1

// will hold all process info
struct FullprocessInfo {
    int pid;
    char state[16];
    int ppid;
    int sz;
    int nfd;
    int nrswitch;
};


void print_table_header()
{
    printf(STDOUT, "PID\tSTATE\t\tPPID\tSZ\tNFD\tNRSWITCH\n");
}


int main()
{
    // get number of processes
    const int num_of_processes = getNumProc();
    const int max_pid = getMaxPid();

    // will hold the process info
    struct processInfo *p = malloc(sizeof(struct processInfo));
    // will hold the full process info including pid and state in string format
    struct FullprocessInfo *fp = malloc(sizeof(struct FullprocessInfo));

    memset(p, 0, sizeof(struct processInfo));
    memset(fp, 0, sizeof(struct FullprocessInfo));

    printf(STDOUT, "Total number of active processes: %d\n", num_of_processes);
    printf(STDOUT, "Maximum PID: %d\n", max_pid);

    // print table header
    print_table_header();

    // iterate over all processes
    for (int pid = 1; pid <= max_pid; pid++)
    {
        // get process info
        if(getProcInfo(pid, p)!=0) // process doesn't exist
        {
            continue;
        }
        else if (p->state == 0) // process is unused
        {
            continue;
        }

        // fill fp
        fp->pid = pid;
        fp->sz = p->sz;
        fp->nfd = p->nfd;
        fp->nrswitch = p->nrswitch;
        if(pid == 1) // PPID of init is 0
        {
            fp->ppid = 0;
        }
        else
        {
            fp->ppid = p->ppid;
        }

        // get state
        if (p->state == 1)
            strcpy(fp->state, "EMBRYO");
        else if (p->state == 2)
            strcpy(fp->state, "SLEEPING");
        else if (p->state == 3)
            strcpy(fp->state, "RUNNABLE");
        else if (p->state == 4)
            strcpy(fp->state, "RUNNING");
        else if (p->state == 5)
            strcpy(fp->state, "ZOMBIE");
        else
        {
            printf(STDOUT, "Error: unknown state\n");
            exit();
        }

        // print process info aligned with table header - according to the process state length
        if (p->state == 1 || p->state == 4 ||  p->state == 5)
            printf(STDOUT, "%d\t%s\t\t%d\t%d\t%d\t%d \n", fp->pid, fp->state, fp->ppid, fp->sz, fp->nfd, fp->nrswitch);
        else if (p->state == 2 || p->state == 3)
            printf(STDOUT, "%d\t%s\t%d\t%d\t%d\t%d \n", fp->pid, fp->state, fp->ppid, fp->sz, fp->nfd, fp->nrswitch);
        else
        {
            printf(STDOUT, "Error: unknown state\n");
            exit();
        }
        
        // clear value from p and fp
        memset(p, 0, sizeof(struct processInfo));
        memset(fp, 0, sizeof(struct FullprocessInfo));
    }
    
    // free allocated memory
    free(p);
    free(fp);

    exit();
}
