char f1(void)
{
	return -1;
}

short f2(void)
{
	return -1;
}

int f3(void)
{
	return -1;
}

long f4(void)
{
	return -1;
}

int main(void)
{
	if (f1() >= 0)
		return 1;
	if (f2() >= 0)
		return 2;
	if (f3() >= 0)
		return 3;
	if (f4() >= 0)
		return 4;
	return 0;
}
