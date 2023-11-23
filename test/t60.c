int f(void)
{
	extern int a;
	return a;
}

int a;

int main(void)
{
	if (f() != 0)
		return 1;
	a = 1;
	if (f() != 1)
		return 2;
	return 0;
}
