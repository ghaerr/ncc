int main(void)
{
	int i = 0;
	int *p = &i;
	*p |= 4;
	if (i != 4)
		return 1;
	return 0;
}
