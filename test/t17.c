int main(void)
{
	unsigned char c1 = 244;
	char c2 = 244;
	long l = (char) -1;
	if (c2 != (char) c1)
		return 1;
	if (l != -1)
		return 2;
	if ((long) (char) -1 != -1)
		return 3;
	if ((long) (unsigned char) (char) -1 != 255)
		return 4;
	if ((int) ((long) -1 >> 31) != -1)
		return 5;
#ifdef __x86_64__
	if ((int) ((long) -1 >> 32) != -1)
		return 6;
#endif
	return 0;
}
