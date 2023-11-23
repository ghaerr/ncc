/* testing signed multiplication */

int main(void)
{
	long i1 = 2;
	long i2 = 3;
	long i3 = -4;
	long i4 = -5;
	if (i1 * i2 != 6)
		return 1;
	if (i1 * i3 != -8)
		return 2;
	if (i3 * i4 != 20)
		return 3;
	return 0;
}
