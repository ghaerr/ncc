int f(s, i)
	char *s;
	int i;
{
	while (*s++)
		i++;
	return i;
}

int main(void)
{
	if (f("aaa", 1) != 4)
		return 1;
	return 0;
}
