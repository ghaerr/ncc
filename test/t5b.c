int main(void)
{
	int c0 = 0;
	int c1 = 1;
	if (c0 || c0 && c1 || c0)
		return 1;
	if (c0 && (c0 || c1))
		return 2;
	if (c1 != 0 && (c0 || c1))
		return 0;
	return 3;
}
