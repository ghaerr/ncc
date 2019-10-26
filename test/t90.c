int main(void)
{
	char (*p)[10];
	char a[10] = "abcd";
	p = a;
	if (sizeof(p) != sizeof(long))
		return 1;
	if (sizeof(p[0]) != 10)
		return 1;
	if (p[0][2] != 'c')
		return 2;
	return 0;
}
