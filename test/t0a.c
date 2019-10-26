int a[10];

int *g(void)
{
	return a;
}

int main(void)
{
	int i;
	for (i = 0; i < 10; i++)
		a[i] = i;
	return g()[3] != 3;
}
