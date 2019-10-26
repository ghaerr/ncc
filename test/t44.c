struct t {
	int a;
};

int g(void)
{
	return 1;
}

int main(void)
{
	struct t t;
	struct t *p = &t;
	p->a = g();
	if (t.a != 1)
		return 1;
	return 0;
}
