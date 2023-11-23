struct dat {
	int a[16];
};

static int a = 0;

static struct dat *f(void)
{
	a++;
	return 0;
}

int main(void)
{
	int sz = sizeof(f()->a);
	if (sizeof(f()->a) != 16 * sizeof(int))
		return 1;
	if (sz != 16 * sizeof(int))
		return 2;
	if (a)
		return 3;
	return 0;
}
