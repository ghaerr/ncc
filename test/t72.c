#define BUF_FILE	0

int main(void)
{
	int type = 2;
	if (type & BUF_FILE)
		return 1;
	return 0;
}
