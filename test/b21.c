int main(void)
{
	int a = 10;
	int b = 10;
	int c = 20;
	if (a < b)
		return 0;
	if (a != b)
		return 1;
	if (a == c)
		return 2;
	if (a > c)
		return 3;
	if (!a)
		return 4;
	return (a + b) != c ? 0 : 21;
}
