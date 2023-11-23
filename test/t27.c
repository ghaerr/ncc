struct t *g(struct t *t)
{
	return t;
}

struct t {
	int a;
	int b;
};

int main(void)
{
	struct t t = {10, 20};
	if (g(&t)->a != 10 || g(&t)->b != 20)
		return 1;
	return 0;
}
