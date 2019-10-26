int g(void)
{
	return 10;
}

static int (*gp)(void) = &g;

int main(void)
{
	if (gp() != 10)
		return 1;
	return 0;
}
