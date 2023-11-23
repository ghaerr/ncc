signed char g(void)
{
	return 254;
}

int main(void)
{
	if (g() != -2)
		return 1;
	return 0;
}
