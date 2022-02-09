#include <xinu.h>
#include <prodcons.h>
#include <stdlib.h>
#include <string.h>
#include <run.h>
int n;
sid32 can_read;
sid32 can_write;
sid32 can_join;
shellcmd xsh_prodcons(int nargs, char *args[])
{
    // Argument verifications and validations
    int count = 200; // local varible to hold count
    can_write = semcreate(1);
    can_read = semcreate(0);
    can_join = semcreate(0);
    if (nargs == 2)
    {
        count = atoi(args[1]);
    }
    else if (nargs > 2)
    {
        fprintf(stderr, "%s: too many arguments", args[0]);
        printf("\n");
        return 0;
    }
    // TODO: check args[1], if present assign value to count

    // create the process producer and consumer and put them in ready queue.
    // Look at the definations of function create and resume in the system folder for reference.
    resume(create(producer, 1024, 20, "producer", 1, count));
    resume(create(consumer, 1024, 20, "consumer", 1, count));
    wait(can_join);
    signal(can_exit);
    return (0);
}