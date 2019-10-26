#define M(a)		((a) + 1) /*((a) + 1)*/

int main(void)
{
	if (M(2) != 3)
		return 1;
	return 0;
}
