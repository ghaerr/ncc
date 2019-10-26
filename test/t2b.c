void g(char *a[])
{
	a[1] = "d";
}

int main(void)
{
	char *a[3] = {"a", "b", "c"};
	g(a);
	if (a[1][0] != 'd')
		return 1;
	return 0;
}
