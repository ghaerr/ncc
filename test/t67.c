int main(void)
{
	int a = 1;
	if ((sizeof a + 1) != sizeof(a) + 1)
		return 1;
	return 0;
}
