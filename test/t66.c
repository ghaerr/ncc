int main(void)
{
	int t = 1;
	int f = 0;
	int v = 3;
	if ((t ? v : 2) != 3)
		return 1;
	if ((f ? v : 2) != 2)
		return 1;
	if ((t ? 2 : v) != 2)
		return 1;
	if ((f ? 2 : v) != 3)
		return 1;
	return 0;
}
