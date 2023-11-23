struct rstate {
	int mark[64 * 2];
	int pc;
	char *s;
	char *o;
	int flg;
};

static int flg(struct rstate *rs)
{
	struct rstate x = *rs;
	return x.flg;
}

int main(void)
{
	struct rstate y;
	y.flg = 0x11223344;
	if (flg(&y) != 0x11223344)
		return 1;
	if (y.flg != 0x11223344)
		return 2;
	return 0;
}
