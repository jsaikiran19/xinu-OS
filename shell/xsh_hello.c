/* xsh_hello.c  - xsh_hello */

#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <run.h>
/*------------------------------------------------------------------------
 * xsh_hello - displays welcome message
 *------------------------------------------------------------------------
 */
shellcmd xsh_hello(int nargs, char *args[]) {

	
	

	/* Check argument count */

    if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Usage: %s\n\n", args[0]);
		printf("Description:\n");
		printf("\tDisplays a welcome message\n");
        printf("Options:\n");
		printf("\tname...\ttakes exactly one name\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	if (nargs > 2) {
		fprintf(stderr, "%s: too many arguments\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
			args[0]);
		return 1;
	}

    if (nargs < 2) {
        fprintf(stderr, "%s: too few arguments\n",args[0]);
        fprintf(stderr, "Try '%s -- help' for more information\n",args[0]);
        return 1;
    }

	if (nargs == 2) {
		printf("Hello %s, Welcome to the world of Xinu!!",args[1]);
        printf("\n");
	}
	signal(can_exit);
	return 0;
}
