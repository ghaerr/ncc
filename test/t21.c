int g(int p1, int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9)
{
	return p1 == 1 && p2 == 2 && p3 == 3 && p4 == 4 && p5 == 5 &&
		p6 == 6 && p7 == 7 && p8 == 8 && p9 == 9;
}

int main(void)
{
	return !g(1, 2, 3, 4, 5, 6, 7, 8, 9);
}
