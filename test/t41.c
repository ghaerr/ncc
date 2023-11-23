int main(void)
{
	unsigned long l = 1;
	if (l != 1)
		return 1;
	if (!l != 0)
		return 2;
	if (!!l != 1)
		return 3;
	if (!l)
		return 4;
	if (!!!l)
		return 5;
	return 0;
}
