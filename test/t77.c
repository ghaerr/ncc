int f(int a[][2])
{
	a[0][0] = 0;
	a[0][1] = 1;
	a[1][0] = 2;
	a[1][1] = 3;
}

int main(void)
{
	int a[2][2];
	f(a);
	if (a[0][0] != 0 || a[0][1] != 1 || a[1][0] != 2 || a[1][1] != 3)
		return 1;
	return 0;
}
