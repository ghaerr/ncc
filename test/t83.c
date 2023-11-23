#define C1(x)		(!((x) & ~0x7f))
#define C2(x)		((x) < 0x80)

int main(void)
{
	int i;
	for (i = 0; i < 0x1000; i++)
		if (C1(i) != C2(i))
			return 1;
	return 0;
}
