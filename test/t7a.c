int g(int x)
{
	int a, b, c, d, e, f;
	int ret = 0;
	a = x;
	b = x;
	c = x;
	d = x;
	e = x;
	f = x;
	if (x > 0 && g(x - 1))
		return 1;
	if (a != x || b != x || c != x || d != x || e != d || f != d)
		return 1;
	return ret;
}

int main(void)
{
	return g(16);
}
