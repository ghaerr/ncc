int main(void)
{
	char *s = "\33\xf\x11";
	if ('\0' != 0)
		return 1;
	if (s[0] != 3 * 8 + 3)
		return 2;
	if (s[1] != 15 || s[2] != 1 * 16 + 1)
		return 3;
	return 0;
}
