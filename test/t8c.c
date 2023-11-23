int main(void)
{
	unsigned n = 0x10000000;
	int m = 0x1000;
	unsigned a = 0x80000001;
	unsigned b = 0x80000002;
	if ((n * m / m) != 0)
		return 1;
	if (a + b != 0x00000003)
		return 2;
	if (sizeof(a + b) != 4)
		return 3;
	return 0;
}
