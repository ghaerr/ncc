int main(void)
{
	int i = 0;
	while (++i < 10) {
		if (i == 5)
			goto out;
		if (i == 3)
			continue;
	}
out:
	return i != 5;
}
