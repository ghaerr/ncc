#define A()		(10)
#define B(a)		(10)

int main(void)
{
	int c10 = 10;
	if (A() != c10)
		return 1;
	if (B(0) == c10)
		return 0;
	return 2;
}
