int mstrcmp(char *s, char *r)
{
	while (*s && *s == *r)
		s++, r++;
	return *s - *r;
}

int main(void)
{
	char *s1 = "abc";
	char *s2 = "123";
	char *s3 = "abc";
	if (!mstrcmp(s1, s2))
		return 1;
	if (!mstrcmp(s2, s3))
		return 2;
	if (mstrcmp(s1, s3))
		return 3;
	return 0;
}
