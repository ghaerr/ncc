int main(void)
{
	int a[10][10];
	int i, j;
	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			a[i][j] = i * 10 + j;
	for (i = 0; i < 10; i++)
		for (j = 0; j < 10; j++)
			if (a[i][j] != i * 10 + j)
				return j;
	return 0;
}
