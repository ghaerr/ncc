#define TABITEMS		0x1000

int hash(char *s)
{
	unsigned h = 0x12345678;
	while (*s) {
		h ^= (h >> ((h & 0xf) + 1));
		h += *s++;
		h ^= (h << ((h & 0xf) + 5));
	}
	h &= (TABITEMS - 1);
	return h ? h : 1;
}

int main(void)
{
	hash("__STDC__");
	hash("__i386__");
	return 0;
}
