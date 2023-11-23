int main(void)
{
	long l = -1;
	char c = 254;
	if (l >> 28 != -1)
		return 1;
	if ((long) (unsigned char) c != 254)
		return 2;
	if ((unsigned long) (c + 1) != -1)
		return 3;
	if ((unsigned long) (c * 10) != -20)
		return 4;
	if (((unsigned long) (unsigned int) -1 >> 16) != 0xffff)
		return 5;
	return 0;
}
