int main(void)
{
	int i;
	for (i = 0; i < 10; i++) {
		if (i == 3)
			continue;
		if (i >= 3)
			break;
	}
	if (i != 4)
		return 1;
	i = 0;
	while (i < 10) {
		i++;
		if (i == 3)
			continue;
		if (i >= 3)
			break;
	}
	if (i != 4)
		return 1;
	return 0;
}
