int main(void)
{
	int a = 1;
	{
		int a = 2;
		if (a != 2)
			return 1;
	}
	if (a != 1)
		return 2;
	return 0;
}
