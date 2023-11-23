static int f(int a, int b, int c, int d)
{
	int *p = &d;
	return p ? d : 0;
}

int main(void)
{
	return f(1, 2, 3, 4) != 4;
}
