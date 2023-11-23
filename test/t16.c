int main(void)
{
	int i;
	for (i = 0; i < 10; i++) {
		switch (i) {
		case 0:
			if (i != 0)
				return 1;
			break;
		case 5:
		case 5 + 1:
			if (i != 5 && i != 6)
				return 2;
			break;
		default:
			if (i == 0 || i == 5 || i == 6)
				return 3;
		}
	}
	if (i != 10)
		return 4;
	return 0;
}
