struct t {
	int a[2];
	int b;
};

int main(void)
{
	struct t t[2];
	if (sizeof(t) != 6 * 4)
		return 1;
	if (sizeof(t[0]) != 3 * 4)
		return 1;
	if (sizeof(t[0].a) != 2 * 4)
		return 1;
	if (sizeof(t[0].b) != 4)
		return 1;
	if (sizeof(t[0].a[0]) != 4)
		return 1;
	if (sizeof(struct t) != 12)
		return 1;
	return 0;
}
