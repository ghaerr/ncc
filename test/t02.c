int main(void)
{
	int a = 1;
	int b = 7;
	a += 5;
	a += 5;
	a -= b;
	return a != 4;
}
