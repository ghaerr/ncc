int main(void)
{
	int a = 12;
	int *b;
	b = &a;
	*b = *b + 1;
	return *b;
}
