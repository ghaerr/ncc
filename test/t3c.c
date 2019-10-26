int g1(int a)
{
	return a;
}

int g2(int a, int b)
{
	return a + b;
}

int main(void)
{
	if (g2(g1(1), g2(g1(2), g1(3))) != 6)
		return 1;
	return 0;
}
