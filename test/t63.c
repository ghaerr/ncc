int main(void)
{
	int a = 5;
	int r = 0;
	switch (a) {
	case 1:
		r |= 0x01;
	case 3:
		r |= 0x02;
	case 5:
		r |= 0x04;
	case 7:
		r |= 0x08;
	default:
		r |= 0x10;
	}
	if (r != 0x1c)
		return 1;
	r = 0;
	switch (a) {
	case 1:
		r |= 0x01;
	default:
		r |= 0x02;
	case 7:
		r |= 0x08;
		break;
	}
	if (r != 0x0a)
		return 2;
	return 0;
}
