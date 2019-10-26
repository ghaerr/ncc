int main(void)
{
	char *a = "a" "b"  "c";
	if (a[0] != 'a' || a[1] != 'b' || a[2] != 'c' || a[3] != 0)
		return 1;
	return 0;
}
