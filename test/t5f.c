static int a;

int main(void)
{
	int f(void);
	if (f() != 0)
		return 1;
	a = 1;
	if (f() != 1)
		return 2;
	return 0;
}

int f(void)
{
	return a;
}
