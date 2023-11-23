int main(void)
{
	char a[3] = "ab";
	if (sizeof(a) != 3)
		return 1;
	if (a[0] == 'a' && a[1] == 'b' && a[2] == 0)
		return 0;
	return 2;
}
