struct t {
	int a[3];
	int b;
	int c;
};

static int a[3] = {1, 2};

int main(void)
{
	static struct t t = {{10, 20}, 30, 40};
	if (a[0] != 1 || a[1] != 2 || a[2] != 0)
		return 1;
	if (t.a[0] != 10 || t.a[1] != 20 || t.a[2] != 0)
		return 2;
	if (t.b != 30 || t.c != 40)
		return 3;
	return 0;
}
