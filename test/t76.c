int main(void)
{
	int a = 1;
	int b = 2;
	a += (a += 2, b = 4);
	return a - 7;
}
