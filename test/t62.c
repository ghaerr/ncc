int main(void)
{
	unsigned char a[10];
	int i;
	for (i = 0; i < sizeof(a); i++)
		a[i] = i;
	a[5] += 2;
	for (i = 0; i < sizeof(a); i++)
		if (i != 5 && a[i] != i)
			return i;
	if (a[5] != 7)
		return 5;
	return 0;
}
