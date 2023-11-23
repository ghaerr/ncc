int main(void)
{
	int a[3];
	a[0] = 1;
	*(a + a[0]) = 2;
	a[2] = 12;
	a[1] = (a + 3) - (a + 1);
	return *a + a[1] + *(a + 2);
}
