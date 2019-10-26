static int files[16][1024];
static int nfiles;
static int cfile;

static int f(char *s)
{
	return 0;
}

int main(void)
{
	f(files[nfiles++]);
	return 0;
}
