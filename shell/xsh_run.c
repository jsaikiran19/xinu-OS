#include <xinu.h>
#include <stdlib.h>
#include <string.h>

#include <shprototypes.h>

shellcmd xsh_run(int nargs, char *args[])
{

    struct cmdent supportedFunctions[] = {
        {"hello", FALSE, xsh_hello},
        {"list", FALSE, NULL},
        {"prodcons",FALSE, xsh_prodcons}
    };
    int i;
    int n = (int)sizeof(supportedFunctions) / (int)sizeof(supportedFunctions[0]);

    if ((nargs == 1) || (strncmp(args[1], "list", 4) == 0))
    {
        for (i = 0; i < n; i++)
        {
            printf("%s\n", supportedFunctions[i].cname);
        }
        return OK;
    }
    args++;
    nargs--;

    for (i = 0; i < n; i++)
    {
        if (strncmp(args[0], supportedFunctions[i].cname, 13) == 0)
        {
            resume(create((void *)supportedFunctions[i].cfunc, 4096, 20, supportedFunctions[i].cname, 2, nargs, args));
            break;
        }
    }

    return (0);
}