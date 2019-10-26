int main(void)
{
	int ret = 0;
	int a = 10;
	int b = 10 + (1 < 2);
	if (a < b && a == 10)
		ret++;
	if (a == b || a == 11)
		ret += 10;
	if (a == b || a == 10)
		ret++;
	return (ret + !0) != 3;
}
