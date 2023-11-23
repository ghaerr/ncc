#define SZ		(1 << 18)

int main(void)
{
	int a[20];
	int i = 10;
	int j;
	a[i] = 5;
	j = (unsigned char) a[i];
	if (j != 5)
		return 1;
	return 0;
}
