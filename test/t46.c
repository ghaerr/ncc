enum {
	TEN = 10,
	TWENTY = 20,
	THIRTY = 30
};
int g(void)
{
	return 30;
}

int main(void)
{
	switch (g()) {
	case TEN:
		return 1;
	case TWENTY:
		return 2;
	case THIRTY:
		return 0;
	default:
		return 3;
	}
	return 0;
}
