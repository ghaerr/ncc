int g1(void)
{
	return 10;
}

int g2(int (*x)(void))
{
	return x();
}

int main(void)
{
	if (g2(g1) != 10)
		return 1;
	return 0;
}
