#include <xinu.h>
#include <stdlib.h>
#include <string.h>
#include <run.h>
#include <shprototypes.h>
#include <prodcons_bb.h>
#include <future_prodcons.h>
#include <ctype.h>
#include <future.h>
#include <stream.h>
#include <fs.h>
sid32 can_exit;
sid32 can_produce;
sid32 can_consume;
sid32 can_exit_prodcons_bb;
int arr_q[5];
struct command
{
    char *name;
    void (*cfunc)(int32, char *[]);
};
int write_index, read_index, total_count;
void list(int nargs, char *args[]);
void hello(int nargs, char *args[]);
void prodcons(int nargs, char *args[]);
void prodcons_bb(int nargs, char *args[]);
void future_prodcons(int nargs, char *args[]);
void stream_proc_tscdf(int nargs, char *args[]);
void stream_proc_future(int nargs, char *args[]);
int fstest(int nargs, char *args[]);
int fs_test(int nargs, char *args[]);

char *supportedFunctions[] = {
    "hello", "list", "prodcons", "prodcons_bb"};

const struct command run_commands[] = {
    {"futest",
     future_prodcons},
    {"hello",
     hello},
    {"list",
     list},
    {"prodcons",
     prodcons},
    {"prodcons_bb",
     prodcons_bb},
    {"tscdf",
     stream_proc_tscdf},
    {"tscdf_fq",
     stream_proc_future},
    {"fstest",
     fs_test}
     };

int size = (int)sizeof(run_commands) / (int)sizeof(run_commands[0]);
shellcmd xsh_run(int nargs, char *args[])
{

    can_exit = semcreate(0);
    can_produce = semcreate(1);
    can_consume = semcreate(0);
    can_exit_prodcons_bb = semcreate(0);
    write_index = 0;
    if ((nargs == 1))
    {
        list(nargs, args);
        return 0;
    }

    args++;
    nargs--;
    bool8 cmd_found = FALSE;
    for (int i = 0; i < size; i++)
    {
        int cmd_length = strlen(run_commands[i].name) + 4;
        if (strncmp(args[0], run_commands[i].name, cmd_length) == 0)
        {
            run_commands[i].cfunc(nargs, args);
            cmd_found = TRUE;
        }
    }
    if (!cmd_found)
    {
        list(nargs, args);
    }

    return (0);
}

void list(int nargs, char *args[])
{
    for (int i = 0; i < size; i++)
    {
        printf("%s\n", run_commands[i].name);
    }
}

void hello(int nargs, char *args[])
{
    if (nargs != 2)
    {
        printf("Syntax: run hello name\n");
    }
    else
    {
        resume(create(xsh_hello, 4096, 20, "hello", 2, nargs, args));
        wait(can_exit);
    }
}

void prodcons(int nargs, char *args[])
{
    if (nargs <= 2)
    {
        resume(create(xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));
        wait(can_exit);
    }
    else
    {
        printf("Syntax: run prodcons [counter]\n");
    }
}

void prodcons_bb(int nargs, char *args[])
{
    args++;
    nargs--;

    if (nargs != 4)
    {
        printf("Syntax: run prodcons_bb [# of producer processes] [# of consumer processes] [# of iterations the producer runs] [# of iterations the consumer runs]");
        printf("\n");
        signal(can_exit);
        return;
    }
    int n_producers = atoi(args[0]);
    int prod_iterations = atoi(args[2]);
    int n_consumers = atoi(args[1]);
    int cons_iterations = atoi(args[3]);

    if (n_producers * prod_iterations != n_consumers * cons_iterations)
    {
        printf("Iteration Mismatch Error: the number of producer(s) iteration does not match the consumer(s) iteration\n");
    }
    else
    {
        // int prod_args[1] = {prod_iterations};
        // int cons_args[1] = {cons_iterations};
        total_count = n_producers * prod_iterations;
        for (int i = 0; i < n_producers; i++)
        {
            resume(create(producer_bb, 4096, 20, "producer_bb", 2, i, prod_iterations));
        }
        for (int j = 0; j < n_consumers; j++)
        {
            resume(create(consumer_bb, 4096, 20, "consumer_bb", 2, j, cons_iterations));
        }
        wait(can_exit_prodcons_bb);
    }
    signal(can_exit);
    return;
}

int fs_test(int nargs, char *args[])
{
    resume(create((void *)fstest, 4096, 20, "fs_tset", 2, nargs, args));
}

void stream_proc_tscdf(int nargs, char *args[])
{
    resume(create((void *)stream_proc, 4096, 20, "stream_proc", 2, nargs, args));
}

void stream_proc_future(int nargs, char *args[])
{
    resume(create(stream_proc_futures, 4096, 20, "stream_proc_futures", 2, nargs, args));
    wait(can_exit);
}

int check_number(char *s)
{
    int l = strlen(s);
    int i = 0;
    for (i = 0; i < l; i++)
    {
        if (isdigit(s[i]) <= 0)
        {
            return -1;
        }
    }
    return 1;
}

void future_prodcons(int nargs, char *args[])
{
    char errorMsg[] = "Syntax: run futest [-pc [g ...] [s VALUE ...]] | [-pcq LENGTH [g ...] [s VALUE ...]] | [-f NUMBER] | [--free]\n ";
    if (nargs < 2)
    {
        printf(errorMsg);
        return;
    }
    if (strcmp(args[1], "--free") == 0)
    {
        if (nargs > 2)
        {
            printf(errorMsg);
            return;
        }
        resume(create(future_free_test, 2048, 20, "free", 2, nargs, args));
        return;
    }
    int i = 2;
    if (strcmp(args[1], "-pc") == 0)
    {
        if (nargs <= 3)
        {
            printf(errorMsg);
            return;
        }
        while (i < nargs)
        {

            if ((strcmp(args[i], "g") != 0) && (strcmp(args[i], "s") != 0) && (check_number(args[i]) == -1))
            {
                printf(errorMsg);
                return;
            }
            else if ((strcmp(args[i], "g") == 0) && (check_number(args[i + 1]) == 1))
            {
                printf(errorMsg);
                return;
            }
            else if ((strcmp(args[i], "s") == 0) && (check_number(args[i + 1]) == -1))
            {
                printf(errorMsg);
                return;
            }
            else if ((check_number(args[i]) == 1) && (strcmp(args[i - 1], "s") != 0))
            {

                printf(errorMsg);
                return;
            }

            i++;
        }
    }
    else if (strcmp(args[1], "-f") == 0)
    {
        if (check_number(args[2]) != 0)
        {
            printf(errorMsg);
            return;
        }
        future_fib(nargs, args);
        return;
    }
    else if (strcmp(args[1], "-pcq") == 0)
    {
        if (nargs <= 3)
        {
            printf(errorMsg);
            return;
        }
        int i = 3;
        while (i < nargs)
        {
            if (strcmp(args[i], "g") != 0 && strcmp(args[i], "s") != 0 && check_number(args[i]) == -1)
            {
                printf(errorMsg);
                return;
            }
            if (strcmp(args[i], "s") == 0 && check_number(args[i + 1]) == -1)
            {
                printf(errorMsg);
                return;
            }
            if (check_number(args[i]) == 1 && strcmp(args[i - 1], "s") != 0)
            {
                printf(errorMsg);
                return;
            }
            i++;
        }
    }
    print_sem = semcreate(1);
    future_t *future_exclusive;
    if (strcmp(args[1], "-pcq") == 0)
    {
        if (check_number(args[2]) != 1)
        {
            printf(errorMsg);
            return;
        }
        future_exclusive = future_alloc(FUTURE_QUEUE, sizeof(int), atoi(args[2]));
    }
    else
    {
        future_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
    }
    char *val;

    int num_args = i; // keeping number of args to create the array
    i = 2;            // reseting the index
    val = (char *)getmem(num_args);

    // Iterate again through the arguments and create the following processes based on the passed argument ("g" or "s VALUE")
    while (i < nargs)
    {
        if (strcmp(args[i], "g") == 0)
        {
            char id[10];
            sprintf(id, "fcons%d", i);
            resume(create(future_cons, 2048, 20, id, 1, future_exclusive));
        }
        if (strcmp(args[i], "s") == 0)
        {
            i++;
            uint8 num = atoi(args[i]);
            val[i] = num;
            resume(create(future_prod, 2048, 20, "fprod", 2, future_exclusive, &val[i]));
            sleepms(5);
        }
        i++;
    }
    sleepms(100);
    future_free(future_exclusive);
}