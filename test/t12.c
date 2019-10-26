enum a {
	a0,
	a1,
	a2 = 5 + 5,
	a3,
	a4 = -2,
	a5,
};

int main(void)
{
	if (a0 != 0 || a1 != 1 || a2 != 10 || a3 != 11 || a4 != -2 || a5 != -1)
		return 1;
	return 0;
}
