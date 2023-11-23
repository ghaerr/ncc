int main(void)
{
	char *s = "\2\12\123\x32";
	if (s[0] != 02 || s[1] != 012 || s[2] != 0123)
		return 1;
	if (s[3] != 0x32)
		return 2;
	return 0;
}
