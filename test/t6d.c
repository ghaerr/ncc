int f1(void)
{
	return 1;
}

int f2(void)
{
	return 2;
}

int main(void)
{
	int c = 0;
	if ((c ? f1 : f2)() != 2)
		return 1;
	c = 1;
	if ((c ? f1 : f2)() != 1)
		return 2;
	return 0;
}
