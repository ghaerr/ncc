#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAXARGS		512
#define LDOPTS		"lLsgmpe"	/* neatld options */
#define CCOPTS		"cIOEDW"	/* neatcc options */
#define AROPTS		"IDOlLome"	/* options with an argument */

static void die(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void display(char *str, char **args)
{
    printf("%s", str);
    while (*args) {
        printf("%s ", *args++);
    }
    printf("\n");
}

int main(int argc, char *argv[], char *envp[])
{
	char *ccargs[MAXARGS];		/* neatcc options */
	char *ldargs[MAXARGS];		/* neatld options */
	int opt[MAXARGS];		/* opt[i] is one if argv[i] is an option */
	int optarg[MAXARGS];		/* argv[i + 1] is an argument of argv[i] */
	int ccargc = 0;			/* number of neatcc options */
	int ldargc = 0;			/* number of neatld options */
	int nold = 0;			/* compile only */
	int i;
	display("NEATCC ARGS: ", argv);
	if (argc < 2)
		die("neatcc: ncc/nld wrapper\n");
	/* looking for options that prevent linking + initialize opt[] and optarg[] */
	for (i = 1; i < argc; i++) {
		opt[i] = argv[i][0] == '-' ? argv[i][1] : 0;
		optarg[i] = opt[i] > 0 && strchr(AROPTS, opt[i]) && !argv[i][2];
		nold = nold || opt[i] == 'c' || opt[i] == 'E';
	}
	/* initialize compiler options */
	ccargs[ccargc++] = NCC;
	ccargs[ccargc++] = "-Dfloat=long";
	ccargs[ccargc++] = "-Ddouble=long";
	ccargs[ccargc++] = "-D__extension__=";
	ccargs[ccargc++] = "-I" NLC;
	for (i = 1; i < argc; i += 1 + optarg[i]) {
		if (opt[i] && (strchr(CCOPTS, opt[i]) || (nold && opt[i] == 'o'))) {
			ccargs[ccargc++] = argv[i];
			if (optarg[i])
				ccargs[ccargc++] = argv[i + 1];
		}
	}
	/* invoke neatcc for every .c file */
	for (i = 1; i < argc; i += 1 + optarg[i]) {
		char *arg = argv[i];
		char *end = strchr(arg, '\0');
		if (!opt[i] && arg + 2 < end && end[-2] == '.' && end[-1] == 'c') {
			int st;
			ccargs[ccargc] = arg;
			ccargs[ccargc + 1] = NULL;
			display("+", ccargs);
			if (fork() == 0) {
				execve(ccargs[0], ccargs, envp);
				die("neatcc: could not find ncc\n");
				return 1;
			}
			if (wait(&st) < 0 || WEXITSTATUS(st))
				return 1;
			end[-1] = 'o';		/* for linker */
		}
	}
	/* invoke neatld */
	if (!nold) {
		ldargs[ldargc++] = NLD;
		for (i = 1; i < argc; i += 1 + optarg[i]) {
			if (!opt[i] || !strchr(CCOPTS, opt[i])) {
				ldargs[ldargc++] = argv[i];
				if (optarg[i])
					ldargs[ldargc++] = argv[i + 1];
			}
		}
		ldargs[ldargc++] = NLC "/start.o";
		ldargs[ldargc++] = NLC "/libc.a";
		ldargs[ldargc] = NULL;
		display("+", ldargs);
		execve(ldargs[0], ldargs, envp);
		die("neatcc: could not find nld\n");
	}
	return 0;
}
