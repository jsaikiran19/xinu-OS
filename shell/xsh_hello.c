/* xsh_date.c - xsh_date */

#include <xinu.h>
#include <string.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * xsh_date - obtain and print the current month, day, year, and time
 *------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char *args[]) {

	
	

	/* Check argument count */

	if (nargs > 2) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
			args[0]);
		return 1;
	}

    if (nargs < 2) {
        fprintf(stderr, "%s: too few arguments\n",args[0]);
        fprintf(stderr, "Try '%s -- help' for more information\n",args[0]);
        return 1
    }

	if (nargs == 2) {
		printf("Hello %s, Welcome to the world of Xinu!!",args[1]);
	}
	return 0;
}
