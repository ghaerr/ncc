int main(void)
{
	int off = -0x88;
	if (off == (char) off)
		return 1;
	return 0;
}
