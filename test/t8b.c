/*
 * The following requires operations to be performed using int values
 * and not using values with architecture word size.
 */
int main(void)
{
	unsigned n = 0xffffff00;
	int shift = 24;
	return (n << shift) != 0;
}
