int main(void)
{
	char *s = "ab";
	void *p = s;
	if (*(char *) p != 'a' || *(char *) ((char *) p + 1) != 'b')
		return 1;
	if (sizeof(*(char *) p) != 1)
		return 2;
	return 0;
}
