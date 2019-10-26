struct t {
	int c[3];
	long a;
	long b;
};

int main(void)
{
	struct t t1, t2;
	int b[3];
	t1.a = 10;
	t1.b = 20;
	t1.c[0] = 30;
	t1.c[1] = 40;
	t1.c[2] = 50;
	t2 = t1;
	if (t2.a != 10 || t2.b != 20)
		return 1;
	if (t2.c[0] != 30 || t2.c[1] != 40 || t2.c[2] != 50)
		return 1;
	return 0;
}
