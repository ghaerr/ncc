int g(void)
{
	return 3;
}

int main(void)
{
	int (*h1)(void) = g;
	int (*h2)(void) = &g;
	int (**h3)(void) = &h1;
	if (h1() != 3 || (*h1)() != 3)
		return 1;
	if (h2() != 3 || (*h2)() != 3)
		return 2;
	if ((*h3)() != 3 || (**h3)() != 3)
		return 3;
	return 0;
}
