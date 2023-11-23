#define B		"//"

int main(void)
{
	char *s = "a" B;
	if (s[0] != 'a' || s[1] != '/' || s[2] != '/' || s[3] != '\0')
		return 1;
	return 0;
}
