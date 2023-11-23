int main(void)
{
	int t = 1, f = 0;
	int a = 1;
	return (t ? f ? a * 2 : a * 3 : f ? 5 : 7) != 3;
}
