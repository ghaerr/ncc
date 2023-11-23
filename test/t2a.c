typedef struct t1 {
	int a[3];
} t1;

typedef struct {
	int a[4];
} t2;

int main(void)
{
	t1 x1;
	t2 x2;
	if (sizeof(x1) != 12)
		return 1;
	if (sizeof(x2) != 16)
		return 2;
	x2.a[2] = 10;
	if (x2.a[2] != 10)
		return 3;
	return 0;
}
