int main(void)
{
	int v = 1;
	if ((1 ? v : 2) != 1)
		return 1;
	if ((0 ? v : 2) != 2)
		return 1;
	if ((1 ? 2 : v) != 2)
		return 1;
	if ((0 ? 2 : v) != 1)
		return 1;
	return 0;
}
