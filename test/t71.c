static struct tmp {
	long addr;
	unsigned loc;
	unsigned bt;
} tmp[2];

int main(void)
{
	tmp[0].addr = 1;
	tmp[0].loc = 1;
	tmp[0].bt = 1;
	tmp[1].addr = 2;
	tmp[1].loc = 2;
	tmp[1].bt = 2;
	if (tmp[0].addr != 1 || tmp[0].loc != 1 || tmp[0].bt != 1)
		return 1;
	if (tmp[1].addr != 2 || tmp[1].loc != 2 || tmp[1].bt != 2)
		return 2;
	return 0;
}
