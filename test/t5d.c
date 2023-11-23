struct type {
	unsigned bt;
};

static void g(void)
{
}

int f(struct type *t)
{
	int sign = 1;
	int size = 4;
	int done = 0;
	int i = 0;
	g();
	t->bt = size | (sign ? 0x100 : 0);
	return 0;
}

int main(void)
{
	struct type t;
	f(&t);
	if (t.bt != 0x104)
		return 1;
	return 0;
}
