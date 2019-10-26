struct {
	int a;
	int b;
} t;

int main(void)
{
	if (sizeof(t) != 8)
		return 1;
	return 0;
}
