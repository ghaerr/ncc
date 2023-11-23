int a;

int g(void)
{
	return a;
}

int main(void)
{
	a += 2;
	return a != g() || a != 2 ? 1 : 0;
}
