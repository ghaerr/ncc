/* should not optimize the last return if it is nested */
int f(int i)
{
	if (i)
		return i;
}

int main(void)
{
	f(1);
	f(2);
	return 0;
}
