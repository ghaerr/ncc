int main(void)
{
	long x = 1;
	if (x << 31 != 1ul << 31)
		return 1;
	return 0;
}
