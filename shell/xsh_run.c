#include <xinu.h>
#include <stdlib.h>
#include <string.h>

#include <shprototypes.h>

struct cmdent supportedFunctions[] = {
    {"hello", FALSE, xsh_hello},
    {"list", FALSE, NULL},
    {"prodcons", FALSE, xsh_prodcons}};

int count = (int)sizeof(supportedFunctions) / (int)sizeof(supportedFunctions[0]);

void printSupportedFunctions(struct cmdent commands[], int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%s\n", commands[i].cname);
    }
}

shellcmd xsh_run(int nargs, char *args[])
{

    // int i;
    if ((nargs == 1) || (strncmp(args[1], "list", 4) == 0))
    {
        printSupportedFunctions(supportedFunctions, count);
        return OK;
    }

    args++;
    nargs--;

    if (strncmp(args[0], "hello", 13) == 0)
    {
        if (nargs != 2)
        {
            printf("Syntax: run hello name\n");
        }
        else
        {
            resume(create((void *)xsh_hello, 4096, 20, "hello", 2, nargs, args));
        }
    }
    else if (strncmp(args[0], "prodcons", 13) == 0)
    {
        if (nargs > 2)
        {
            printf("Syntax: run prodcons [counter]\n");
        }
        else
        {
            resume(create((void *)xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));
        }
    }
    else
    {
        printSupportedFunctions(supportedFunctions, count);
        return OK;
    }

    return (0);
}