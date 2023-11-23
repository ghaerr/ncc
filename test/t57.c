#define SZ		(1 << 10)

int main(void)
{
	int x[123];
	int a[SZ];
	int i;
	short j;
	i = 0x11223344;
	j = 0x5566;
	if (i != 0x11223344)
		return 1;
	if (j != 0x5566)
		return 2;
	for (i = 0; i < SZ; i++)
		a[i] = i;
	for (i = 0; i < SZ; i++)
		if (a[i] != i)
			return 3;
	return 0;
}
