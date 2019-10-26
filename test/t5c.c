#define ALIGN(x, a)             (((x) + (a) - 1) & ~((a) - 1))

int main(void)
{
	int c7 = 7;
	int c4 = 4;
	if (ALIGN(c7, c4) != 8)
		return 1;
	return 0;
}
