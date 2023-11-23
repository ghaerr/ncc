static int rr = 0;
static int gr = 0;
static int br = 0;
static int rl = 16;
static int gl = 8;
static int bl = 0;

unsigned int g(unsigned char r, unsigned char g, unsigned char b)
{
	return ((r >> rr) << rl) | ((g >> gr) << gl) | ((b >> br) << bl);
}

int main(void)
{
	unsigned char cr = 0x11;
	unsigned char cg = 0x22;
	unsigned char cb = 0x33;
	if (g(0, 0, 0) != 0)
		return 1;
	if (g(255, 255, 255) != 0x00ffffff)
		return 2;
	if (g(0x11, 0x22, 0x33) != 0x00112233)
		return 3;
	if (g(cr, cg, cb) != 0x00112233)
		return 4;
	return 0;
}
