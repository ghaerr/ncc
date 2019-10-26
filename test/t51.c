struct t {
	int f1;
	int f2;
	int f3;
	int f4;
	int f5;
	int f6;
};

int g(int p1, int p2, int p3, int p4)
{
	return p1 + p2 + p3 + p4;
}

int main(void)
{
	struct t t = {1, 1, 2, 2, 3, 3};
	struct t *tp = &t;
	int (*fp)(int a1, ...) = g;
	return fp(tp->f6, tp->f5, tp->f4, tp->f3, tp->f2, tp->f1) != 10;
}
