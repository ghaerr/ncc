int main(void)
{
	char a1[1 ? 10 : 20];
	char a2[5 + 5];
	char a3[(1 << 3) + 2];
	char a4[2 < 1 ? 20 : 10];
	if (sizeof(a1) != 10 || sizeof(a2) != 10)
		return 1;
	if (sizeof(a3) != 10 || sizeof(a4) != 10)
		return 1;
	return 0;
}
