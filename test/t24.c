int main(void)
{
	char a[10] = "ab";
	if (sizeof(a) != 10)
		return 1;
	if (a[0] == 'a' && a[1] == 'b' && a[2] == 0 && a[3] == 0)
		return 0;
	return 2;
}
