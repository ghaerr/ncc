static char *digs = "0123456789abcdef";

void putint(char *s, int n, int base)
{
	int d = 10;
	int i;
	for (i = 0; i < d; i++)
		s[d - i] = digs[n % base];
	s[d] = '\0';
}

int main(void)
{
	char dst[16];
	putint(dst, 2, 10);
	return 0;
}
