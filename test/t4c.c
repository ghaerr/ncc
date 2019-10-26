static int i;
static int a[2];

int main(void)
{
	int j = 10;
	int b[2];
	int *p = a;
	if (i++ != 0 || i != 1)
		return 1;
	if (--j != 9 || j != 9)
		return 2;
	p = a;
	*p++ = 10;
	*p++ = 20;
	if (a[0] != 10 || a[1] != 20)
		return 3;
	p = b + 1;
	*p-- = 30;
	*p-- = 40;
	if (b[0] != 40 || b[1] != 30)
		return 4;
	return 0;
}
