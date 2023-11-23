int slen(char *s)
{
	int n = 0;
	while (*s++)
		n++;
	return n;
}

int main(void)
{
	char *s[] = {"ab", "c"};
	int len[] = {slen(s[0]), slen(s[1])};
	if (len[0] != 2 || len[1] != 1)
		return 1;
	return 0;
}
