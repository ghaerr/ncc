struct t {
	int a;
	int b;
};

int main(void)
{
	struct t t[10];
	int i;
	int sum = 0;
	for (i = 0; i < 10; i++)
		t[i].a = t[i].b = i;
	for (i = 0; i < 10; i++)
		if (t[i].a != i || t[i].b != i)
			return 1;
	return 0;
}
