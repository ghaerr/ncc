int main(void)
{
	char a[] = "abc";
	int b[] = {1, 2, 3, 4};

	if (sizeof(a) != 4)
		return 1;
	if (a[0] != 'a' || a[1] != 'b' || a[2] != 'c' || a[3] != 0)
		return 2;
	if (sizeof(b) != 16)
		return 3;
	if (b[0] != 1 || b[1] != 2 || b[2] != 3 || b[3] != 4)
		return 4;
	return 0;
}
