#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#define PGSIZE		4096
#define PGMASK		(PGSIZE - 1)
#define MSETMAX		4096
#define MSETLEN		(1 << 15)
#define MIN(a, b)	((a) < (b) ? (a) : (b))

/* placed at the beginning of regions for small allocations */
struct mset {
	int refs;	/* number of allocations */
	int size;	/* remaining size */
};

/* placed before each small allocation */
struct mhdr {
	int moff;	/* mset offset */
	int size;	/* allocation size */
};

static struct mset *pool;

static long heaptop =  0x0000000080000000;     /* 2G growing upwards */

static int mk_pool(void)
{
	if (pool && !pool->refs) {
		pool->size = sizeof(*pool);
		return 0;
	}
#ifdef __APPLE__
	pool = mmap(heaptop, MSETLEN, PROT_READ | PROT_WRITE,
				MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
	pool = mmap(NULL, MSETLEN, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
	if (pool == MAP_FAILED) {
        printf("pool malloc %lx, errno %d\n", pool, errno);
		pool = NULL;
		return 1;
	}
    heaptop += MSETLEN;
    if (heaptop & 0xFFFFFFFF00000000) printf("malloc addr > 32 bits\n");
	pool->size = sizeof(*pool);
	pool->refs = 0;
	return 0;
}

void *malloc(long n)
{
	void *m;
	if (!n)
		return NULL;
	if (n >= MSETMAX) {
#ifdef __APPLE__
		m = mmap(heaptop, n + PGSIZE, PROT_READ | PROT_WRITE,
				MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
		m = mmap(NULL, n + PGSIZE, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
		if (m == MAP_FAILED) {
            printf("mmap malloc %lx, errno %d\n", m, errno);
			return NULL;
		}
		*(long *) m = n + PGSIZE;	/* store length in the first page */
        heaptop += (n + PGSIZE + PGSIZE - 1) & ~PGMASK;
        if (heaptop & 0xFFFFFFFF00000000) printf("malloc addr > 32 bits\n");
		return m + PGSIZE;
	}
	if (!pool || pool->size + n + sizeof(struct mhdr) > MSETLEN)
		if (mk_pool())
			return NULL;
	m = (void *) pool + pool->size;
	((struct mhdr *) m)->moff = pool->size;
	((struct mhdr *) m)->size = n;
	pool->refs++;
	pool->size += (n + sizeof(struct mhdr) + 7) & ~7;
	if (!((unsigned long) (pool + pool->size + sizeof(struct mhdr)) & PGMASK))
		pool->size += sizeof(long);
	return m + sizeof(struct mhdr);
}

static long msize(void *v)
{
	if ((unsigned long) v & PGMASK)
		return ((struct mhdr *) (v - sizeof(struct mhdr)))->size;
	return *(long *) (v - PGSIZE);
}

void free(void *v)
{
	if (!v)
		return;
	if ((unsigned long) v & PGMASK) {
		struct mhdr *mhdr = v - sizeof(struct mhdr);
		struct mset *mset = (void *) mhdr - mhdr->moff;
		mset->refs--;
		if (!mset->refs && mset != pool)
			munmap(mset, MSETLEN);
	} else {
		munmap(v - PGSIZE, *(long *) (v - PGSIZE));
	}
}

void *calloc(long n, long sz)
{
	void *r = malloc(n * sz);
	if (r)
		memset(r, 0, n * sz);
	return r;
}

void *realloc(void *v, long sz)
{
	void *r = malloc(sz);
	long osz = msize(v);
	/*printf("REALLOC %lx size %ld to %lx size %ld\n", v, osz, r, sz);*/
	if (r && v) {
		memcpy(r, v, MIN(osz, sz));
		free(v);
	}
	return r;
}
