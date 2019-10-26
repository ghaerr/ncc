static int g1(void)
{
	static int i;
	return i++;
}

static int g2(void)
{
	static int i;
	return i++;
}

int main(void)
{
	if (g1() != 0 || g1() != 1)
		return 1;
	if (g2() != 0 || g2() != 1)
		return 2;
	if (g1() != 2 || g1() != 3)
		return 3;
	return 0;
}
