#define A

int main(void)
{
	int i = 0;
#ifdef A
	i = 10;
#else
	i = 20;
#endif
	if (i != 10)
		return 1;
#ifndef A
	i = 1;
#else
	i = 2;
#endif
	if (i != 2)
		return 2;
	return 0;
}
