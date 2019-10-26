#define A		1
#define B		2

int main(void)
{
	int i = 0;
#if A == B
	i = 10;
#elif A == B / 2
	i = 20;
#else
	i = 30;
#endif
	if (i != 20)
		return 1;
	return 0;
}
