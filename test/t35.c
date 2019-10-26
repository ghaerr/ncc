#define N		10

int main(void)
{
	char a[N];
	if (sizeof(a) != 10 || N != 10)
		return 1;
	return 0;
}
