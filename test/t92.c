#define n_itn		(*nreg(map(".itn")))	/* .it lines left */

int x = 2;

int map(char *x)
{
	return 1;
}

int *nreg(int id)
{
	return &x;
}

int main(void)
{
	int i = --(n_itn);
	return x - 1;
}
