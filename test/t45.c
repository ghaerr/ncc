int g(void)
{
	return 30;
}

int main(void)
{
	int a = 10;
	int b = a + a - a + (a == 20 ? g() : 10);
	if (a != 10 || g() != 30 || b != 20)
		return 1;
	return 0;
}
