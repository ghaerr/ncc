#ifndef NDEBUG
#define assert(expr)            \
    if (!(expr)) {              \
        __assertfailed(#expr);  \
    }
#else
#define assert(expr)
#endif

#define unassert(expr)          \
    if (!(expr)) {              \
        __assertfailed(#expr);  \
    }

void __assertfailed(char *str);
