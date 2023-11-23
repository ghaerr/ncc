static int g(void)
{
	return 0x1234;
}

static void *f(int (*x)(void))
{
	return x;
}

int main(void)
{
	if (g != f(g))
		return 1;
	return 0;
}
