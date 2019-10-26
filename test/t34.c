int main(void)
{
	int i = 0;
	for (;;) {
		if (++i == 10)
			break;
	}
	if (i != 10)
		return 1;
	return 0;
}
