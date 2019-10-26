int main(void)
{
	char a[10];
	long sum = 0;
	int i;
	for (i = 0; i < 10; i++)
		a[i] = i;
	for (i = 0; i < 10; i++)
		sum = sum + a[i];
	return (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9) != sum;
}
