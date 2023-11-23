static char *buf = "((x))";
static int cur = 1;
static int len = 5;

void readarg(void)
{
	int depth = 0;
	while (cur < len && (depth || buf[cur] != ')')) {
		switch (buf[cur++]) {
		case '(':
		case '[':
		case '{':
			depth++;
			break;
		case ')':
		case ']':
		case '}':
			depth--;
			break;
		}
	}
}

int main(void)
{
	readarg();
	if (cur != 4)
		return 1;
	return 0;
}
