int main(void)
{
	unsigned int s1 = 30;
	int s2 = -50;
	if (s1 * 4 != 120)
		return 1;
	if (s2 * 4 != -200)
		return 2;
	if (s1 / 2 != 15)
		return 3;
	if (s2 / 2 != -25)
		return 4;
	if (s2 % 2 != 0)
		return 5;
	if (s2 / 8 != -6)
		return 7;
	if (s2 % 8 != -2)
		return 8;
	if (s2 / -8 != 6)
		return 9;
	if (s2 % -8 != -2)
		return 10;
	if (s1 % 8 != 6)
		return 11;
	return 0;
}
