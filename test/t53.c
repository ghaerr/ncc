/* testing optimized version of mul/div/mod for powers of two */

static unsigned a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

int main(void)
{
	unsigned i;
	i = 5;
	if (a[i] != 5)
		return 1;
	if (i * 16 != 80)
		return 2;
	i = 22;
	if (i / 4 != 5)
		return 3;
	if (i % 4 != 2)
		return 4;
	i = 0x12345678;
	if (i % 0x00010000 != 0x00005678)
		return 4;
	return 0;
}
