int main(void)
{
	long i = 0xffffffff;
	if (i != (unsigned) i)
		return 1;
	return 0;
}
