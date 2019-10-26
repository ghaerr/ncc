struct t {
	int a;
	int b;
	int c;
};

int main(void)
{
	struct t t;
	t.a = 2;
	t.b = t.a + 3;
	t.c = t.a + t.b;
	return t.c != 7;
}
