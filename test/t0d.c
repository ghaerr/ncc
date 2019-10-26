struct t {
	int a[10];
	int n;
};

int main(void)
{
	struct t t;
	struct t *tp = &t;
	int i;
	int sum = 0;
	tp->n = 10;
	for (i = 0; i < tp->n; i++)
		tp->a[i] = i;
	for (i = 0; i < t.n; i++)
		sum += t.a[i];
	return sum != 45;
}
