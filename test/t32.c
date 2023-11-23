int main(void)
{
	long a = 0x10 + 010 + 10;
	if (a != 16 + 8 + 10)
		return 1;
	return 0;
}
