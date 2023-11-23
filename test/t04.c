int main(void)
{
	int a = 8;
	int b = 4;
	return (a & (b | 15)) != a;
}
