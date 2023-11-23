struct t {
	int a;
	int b;
	int c;
};

void g(struct t *tp)
{
	tp->a = 2;
}

int main(void)
{
	struct t t;
	struct t *tp = &t;
	g(tp);
	tp->b = tp->a + 3;
	tp->c = tp->a + t.b;
	return tp->c != 7;
}
