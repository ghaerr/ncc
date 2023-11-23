int g(int i)
{
	return i;
}

int main(void)
{
	int (*p)(int i) = g;
	if (p(10) != 10)
		return 1;
	return 0;
}
