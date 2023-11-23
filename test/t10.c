union t {
	int a[2];
	int b;
};

int main(void)
{
	union t t;
	t.a[0] = 1;
	t.a[1] = 2;
	t.b = 3;
	if (sizeof(t) != 8)
		return 1;
	if (t.a[0] != 3)
		return 1;
	if (t.a[1] != 2)
		return 1;
	if (t.b != 3)
		return 1;
	if (&t.b != &t.a[0])
		return 1;
	return 0;
}
