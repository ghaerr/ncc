#ifdef __x86_64__
int main(void)
{
	char *s1[10];
	char s3[10];
	char (*s2)[10];
	int (*x)(int a);
	if (sizeof(s1) != 80)
		return 1;
	if (sizeof(s2) != 8)
		return 2;
	if (sizeof(x) != 8)
		return 2;
	return 0;
}
#else
int main(void)
{
	char *s1[10];
	char s3[10];
	char (*s2)[10];
	int (*x)(int a);
	if (sizeof(s1) != 40)
		return 1;
	if (sizeof(s2) != 4)
		return 2;
	if (sizeof(x) != 4)
		return 3;
	return 0;
}
#endif
