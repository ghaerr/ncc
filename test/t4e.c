int g1(void)
{
	return 10;
}

int g2(void)
{
	return 20;
}

int main(void)
{
	int (*x1[])(void) = {g1, g2};
	int (*x2[][1])(void) = {{g1}, {g2}};
	if (x1[0]() != 10 || x1[1]() != 20)
		return 1;
	if (x2[0][0]() != 10 || x2[1][0]() != 20)
		return 2;
	return 0;
}
