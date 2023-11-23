/* Reported by Alexey Frunze */
int main(void)
{
	int n = 0X01;
	if (n++, n++, n++, n++, ++n != 6)
		return 1;
	return 0;
}
