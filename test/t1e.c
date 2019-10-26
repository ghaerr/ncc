int g1(void);

int main(void)
{
	if (g1() != 11)
		return 1;
	if (g2() != 21)
		return 2;
	return 0;
}

int g1(void)
{
	return 11;
}

int g2(void)
{
	return 21;
}

