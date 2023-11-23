int main(void)
{
	int i = 0;
	if (1)
		do {
			i = 1;
		} while (0);
	else
		i = 2;
	return i != 1;
}
