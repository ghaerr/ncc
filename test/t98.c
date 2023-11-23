#define S(x)	#x
#define S3(x, y, z)	#x#y /* #x */ #z

int main(void)
{
	char *s = S(abc);
	char *r = S3(a, b, c);
	if (s[0] != 'a' || s[1] != 'b' || s[2] != 'c' || s[3])
		return 1;
	if (r[0] != 'a' || r[1] != 'b' || r[2] != 'c' || r[3])
		return 2;
	return 0;
}
