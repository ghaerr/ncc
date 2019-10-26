int mstrcmp(char *s, char *r)
{
	while (*s && *s == *r)
		s++, r++;
	return *s - *r;
}

int main(void)
{
	if (!mstrcmp("abc", "123"))
		return 1;
	if (!mstrcmp("123", "abc"))
		return 2;
	if (mstrcmp("abc", "abc"))
		return 3;
	return 0;
}
