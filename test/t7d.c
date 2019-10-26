#define LONGSZ		4
#define BT_SZMASK	0x00ff
#define BT_SIGNED	0x0100
#define BT_SZ(bt)	((bt) & BT_SZMASK)

struct tmp {
	long addr;
};

void num_cast(struct tmp *t, unsigned bt)
{
	if (!(bt & BT_SIGNED) && BT_SZ(bt) != LONGSZ)
		t->addr &= ((1l << (long) (BT_SZ(bt) * 8)) - 1);
	if (bt & BT_SIGNED && BT_SZ(bt) != LONGSZ &&
				t->addr > (1l << (BT_SZ(bt) * 8 - 1)))
		t->addr = -((1l << (BT_SZ(bt) * 8)) - t->addr);
}

int main(void)
{
	struct tmp t[10];
	struct tmp *t0 = &t[0];
	struct tmp *t1 = &t[1];
	struct tmp *t2 = &t[2];
	struct tmp *t3 = &t[3];
	num_cast(t0, 4);
	num_cast(t1, 4);
	num_cast(t2, 4);
	num_cast(t3, 4);
	if (t0 != &t[0])
		return 1;
	if (t1 != &t[1])
		return 1;
	if (t2 != &t[2])
		return 1;
	if (t3 != &t[3])
		return 1;
	return 0;
}
