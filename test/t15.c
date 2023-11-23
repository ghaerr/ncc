int main(void)
{
	int i = 0;
	do {
		i++;
	} while (i < 10);
	if (i != 10)
		return 1;
	i = 0;
	do {
		i++;
		if (i == 3)
			continue;
		if (i == 4)
			break;
	} while (i < 10);
	if (i != 4)
		return 1;
	return 0;
}
