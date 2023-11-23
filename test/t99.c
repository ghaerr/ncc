#define T(x)	a##x
#define T3(x, y, z)	a##x##y##z

int main(void)
{
	int aabc = 1;
	if (T(abc) != 1)
		return 2;
	if (T3(a, b, c) != 1)
		return 2;
	return 0;
}
