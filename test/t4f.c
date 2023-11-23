#define TWICE(a)	((a) * 2)

int STH(int a)
{
	return a * 3;
}

#define STH(a)		(STH(a) * 2)

int main(void)
{
	int a = 5;
	if (TWICE(1) != 2 && TWICE(TWICE(1)) != 4)
		return 1;
	if (STH(2) != 12)
		return 2;
	if (TWICE(TWICE(a)) != 20)
		return 3;
	return 0;
}
