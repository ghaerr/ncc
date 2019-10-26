#define ABC	"ABC"

char s[][4] = {"a0", "a1", "a2"};

int main(void)
{
	if (s[0][0] != 'a' || s[0][1] != '0' || s[0][2] != '\0')
		return 1;
	if (s[1][0] != 'a' || s[1][1] != '1' || s[1][2] != '\0')
		return 1;
	if (s[2][0] != 'a' || s[2][1] != '2' || s[2][2] != '\0')
		return 1;
	return 0;
}
