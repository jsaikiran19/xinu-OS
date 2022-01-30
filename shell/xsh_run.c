#include <xinu.h>
#include <stdlib.h>
#include <string.h>

#include <shprototypes.h>

struct cmdent supportedFunctions[] = {
    {"hello", FALSE, xsh_hello},
    {"list", FALSE, NULL},
    {"prodcons", FALSE, xsh_prodcons}};

int size = (int)sizeof(supportedFunctions) / (int)sizeof(supportedFunctions[0]);

shellcmd xsh_run(int nargs, char *args[])
{

    
    if ((nargs == 1) || (strncmp(args[1], "list", 4) == 0))
    {
        for (int i = 0; i < size; i++)
        {
            printf("%s\n", supportedFunctions[i].cname);
        }
        return OK;
    }

    args++;
    nargs--;

    if (strncmp(args[0], "hello", 5) == 0)
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
    else if (strncmp(args[0], "prodcons", 8) == 0)
    {
        if (nargs <= 2)
        {
            resume(create((void *)xsh_prodcons, 4096, 20, "prodcons", 2, nargs, args));
        }
        else
        {
            printf("Syntax: run prodcons [counter]\n");
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            printf("%s\n", supportedFunctions[i].cname);
        }
        return OK;
    }

    return (0);
}