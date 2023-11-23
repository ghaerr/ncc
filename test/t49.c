int main(void)
{
	int i[3] = {2, 0, 3};
	for (i[1] = 9; i[1] >= 0; --i[1])
		;
	if (i[0] != 2 || i[2] != 3)
		return 1;
	return 0;
}
