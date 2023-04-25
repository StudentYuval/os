#include "types.h"
#include "user.h"

// this program will spawn 8 child processes - each with different priority. 

int main(int argc, char *argv[])
{
    int pid;
    int i;
    float sum = 0;

    // spawn 8 child processes
    for (i = 0; i < 8; i++)
    {
        pid = fork();
        if (pid == 0) // child
        {
            // set priority
            setprio(i);

            int start_time = uptime();

            //cpu intensive work - inline assembly with jump
            asm volatile("1: add $1, %0; cmp $10000000000, %0; jle 1b" : "+r"(sum));
            
            int end_time = uptime();
            // print finish message
            printf(1, "Child %d with priority %d finished in %d clock interrupts\n", i, getprio(), end_time - start_time);
            exit();
        }
    }

    // wait for all children to exit
    while(wait()!=-1);

    exit();
}