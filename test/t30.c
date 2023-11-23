static char *a[] = { "a", "b", "c" };

int main(void)
{
	if (a[0][0] != 'a' || a[1][0] != 'b' || a[2][0] != 'c')
		return 1;
	return 0;
}
