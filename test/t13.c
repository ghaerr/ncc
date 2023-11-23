typedef int mint;

int main(void)
{
	mint a;
	if (sizeof(a) != 4 || sizeof(mint) != 4)
		return 1;
	return 0;
}
