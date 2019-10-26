static char saved[2];

int main(void)
{
	if (sizeof(saved) != 2)
		return 1;
	return 0;
}
