#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXARGS	512

static void die(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

int main(int argc, char *argv[], char *envp[])
{
	char *targv[MAXARGS];
	int targc = 0;
	int cc = 0;
	int i;
	if (argc < 2)
		die("neatcc: ncc/nld wrapper\n");
	for (i = 1; i < argc; i++)
		if (argv[i][0] == '-' && argv[i][1] == 'c')
			cc = 1;
	if (cc) {
		targv[targc++] = NCC;
		targv[targc++] = "-Dfloat=long";
		targv[targc++] = "-Ddouble=long";
		targv[targc++] = "-D__extension__=";
		targv[targc++] = "-I" NLC;
		for (i = 1; i < argc; i++)
			targv[targc++] = argv[i];
	} else {
		targv[targc++] = NLD;
		for (i = 1; i < argc; i++)
			targv[targc++] = argv[i];
		targv[targc++] = NLC "/start.o";
		targv[targc++] = NLC "/libc.a";
	}
	targv[targc] = NULL;
	execve(targv[0], targv, envp);
	die("neatcc: could not find ncc/nld\n");
	return 1;
}
