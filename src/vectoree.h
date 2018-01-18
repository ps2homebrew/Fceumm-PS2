#ifndef VECTOREE_H
#define VECTOREE_H

static inline u128 lq(void *addr)
{
    u128 rv;
    asm (
            "lq %[rv], 0(%[addr])\n\t"
            : [rv] "=r" (rv)
            : [addr] "r" (addr)
    );

    return rv;
}

static inline void sq(void *addr, u128 val)
{
    asm volatile (
            "sq %[val], 0(%[addr])\n\t"
            :
            : [val] "r" (val), [addr] "r" (addr)
            : "memory"
    );
}

static inline u128 pand(u128 a, u128 b)
{
    u128 rv;
    asm (
        "pand %[rv], %[a], %[b]\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 pnor(u128 a, u128 b)
{
    u128 rv;
    asm (
            "pnor %[rv], %[a], %[b]\n\t"
            : [rv] "=r" (rv)
            : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 por(u128 a, u128 b)
{
    u128 rv;
    asm (
            "por %[rv], %[a], %[b]\n\t"
            : [rv] "=r" (rv)
            : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 psubb(u128 a, u128 b)
{
    u128 rv;
    asm (
            "psubb %[rv], %[a], %[b]\n\t"
            : [rv] "=r" (rv)
            : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 pcgtb0(u128 a)
{
    u128 rv;
    asm (
        "pcgtb %[rv], %[a], $0\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a)
    );
    return rv;
}

static inline u128 pceqb(u128 a, u128 b)
{
    u128 rv;
    asm (
        "pceqb %[rv], %[a], %[b]\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 pceqb0(u128 a)
{
    u128 rv;
    asm (
        "pceqb %[rv], %[a], $0\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a)
    );
    return rv;
}

static inline u128 pxor(u128 a, u128 b)
{
    u128 rv;
    asm (
        "pxor %[rv], %[a], %[b]\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 ppacb(u128 a, u128 b)
{
    u128 rv;
    asm (
        "ppacb %[rv], %[a], %[b]\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 psllh4(u128 a)
{
    u128 rv;
    asm (
        "psllh %[rv], %[a], 4\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a)
    );
    return rv;
}

static inline u128 psrlh8(u128 a)
{
    u128 rv;
    asm (
        "psrlh %[rv], %[a], 8\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a)
    );
    return rv;
}

static inline u128 pcpyld0(u64 low)
{
    u128 rv;
    asm (
        "pcpyld %[rv], $0, %[low]\n\t"
        : [rv] "=r" (rv)
        : [low] "r" (low)
    );
    return rv;
}

static inline u128 pextlb(u128 a, u128 b)
{
    u128 rv;
    asm (
        "pextlb %[rv], %[a], %[b]\n\t"
        : [rv] "=r" (rv)
        : [a] "r" (a), [b] "r" (b)
    );
    return rv;
}

static inline u128 fill_u128(uint8 val)
{
        u128 rv;
        u128 tmp;
        asm (
                "pcpyld %[tmp], %[val], %[val]\n\t"
                "pcpyh %[tmp], %[tmp]\n\t"
                "ppacb %[rv], %[tmp], %[tmp]\n\t"
                : [rv] "=r" (rv), [tmp] "=r" (tmp)
                : [val] "r" (val)
        );

        return rv;
}

static inline u128 fill_u128_u16(uint16 val)
{
    u128 tmp;
    u128 rv;

    asm (
        "pcpyld %[tmp], %[val], %[val]\n\t"
        "pcpyh %[rv], %[tmp]\n\t"
        : [rv] "=r" (rv), [tmp] "=&r" (tmp)
        : [val] "r" (val)
    );
    return rv;
}

static inline u64 upper_u128(u128 val)
{
    u64 rv;
    asm (
        "pcpyud %[rv], %[val], $0\n\t"
        : [rv] "=r" (rv)
        : [val] "r" (val)
    );

    return rv;
}

static inline u64 lower_u128(u128 val)
{
    return (u64)val;
}

static inline void prefetch(void *addr)
{
    asm volatile(
        "pref 0, 0(%[addr])\n\t"
        :
        : [addr] "r" (addr)
    );
}


#endif //VECTOREE_H
