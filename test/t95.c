int uc_slen(char *s)
{
	return 5;
}

char **uc_chop(char *s, int *n)
{
	char **chrs;
	*n = uc_slen(s);
	return 0;
}

int main(void)
{
	int n;
	uc_chop(0, &n);
	if (n != 5)
		return 1;
	return 0;
}
