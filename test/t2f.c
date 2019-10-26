struct t {
	int a;
	int b;
};

static struct t t1[] = {{10, 20}};

int main(void)
{
	struct t t2[] = {{30, 40}};
	if (t1->a != 10 || t2->b != 40)
		return 1;
	return 0;
}
