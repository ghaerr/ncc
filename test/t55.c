static int digits(unsigned long n, int base)
{
	int i;
	for (i = 0; n; i++)
		n /= base;
	return i ? i : 1;
}

static char *digs = "0123456789abcdef";

static void putint(char *s, unsigned long n, int base)
{
	int d;
	int i;
	d = digits(n, base);
	for (i = 0; i < d; i++) {
		s[d - i - 1] = digs[n % base];
		n /= base;
	}
	s[d] = '\0';
}

static int mstrcmp(char *s, char *r)
{
	while (*s == *r) {
		if (!*s)
			return 0;
		s++;
		r++;
	}
	return *r - *s;
}

int main(void)
{
	char dst[16];
	if (digits(123, 10) != 3)
		return 1;
	putint(dst, 123, 10);
	if (mstrcmp("123", dst))
		return 2;
	putint(dst, 130, 2);
	if (mstrcmp("10000010", dst))
		return 3;
	putint(dst, 0x1234, 16);
	if (mstrcmp("1234", dst))
		return 4;
	putint(dst, 0x8f8f8f8f, 16);
	if (mstrcmp("8f8f8f8f", dst))
		return 5;
	return 0;
}
