int main(void)
{
	int a[3] = {10, 20, 30};
	int *p = a;
	p += 1;
	if (*p != 20)
		return 1;
	p++;
	if (*p != 30)
		return 2;
	return 0;
}
