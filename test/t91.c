int main(void)
{
	char (*p[3])[10];
	char a[10] = "abcd";
	p[1] = a;
	if (sizeof(p) != sizeof(long) * 3)
		return 1;
	if (sizeof(p[1][0]) != 10)
		return 1;
	if (p[1][0][2] != 'c')
		return 2;
	return 0;
}
