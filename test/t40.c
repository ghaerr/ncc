int main(void)
{
	char a[] = "ab";
	int cur = 0;
	if (a[cur++] != 'a')
		return 1;
	if (a[cur++] != 'b')
		return 2;
	if (a[cur++] != '\0')
		return 3;
	return 0;
}
