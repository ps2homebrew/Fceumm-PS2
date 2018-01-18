#ifndef SPROFILE_H
#define SPROFILE_H

enum {
    PCR0_RESERVED = 0u << 5,
    PCR0_PROCESSOR_CYCLE = 1u << 5,
    PCR0_SINGLE_INSTRUCTION_ISSUE = 2u << 5,
    PCR0_BRANCH_ISSUED = 3u << 5,
    PCR0_BTAC_MISS = 4u << 5,
    PCR0_ITLB_MISS = 5u << 5,
    PCR0_INSTRUCTION_CACHE_MISS = 6u << 5,
    PCR0_DTLB_ACCESS = 7u << 5,
    PCR0_NON_BLOCKING_LOAD = 8u << 5,
    PCR0_WBB_SINGLE_REQUEST = 9u << 5,
    PCR0_WBB_BURST_REQUEST = 10u << 5,
    PCR0_CPU_ADDRESS_BUS_BUSY = 11u << 5,
    PCR0_INSTRUCTION_COMPLETED = 12u << 5,
    PCR0_NON_BDS_INSTRUCTION_COMPLETED = 13u << 5,
    PCR0_COP2_INSTRUCTION_COMPLETED = 14u << 5,
    PCR0_LOAD_COMPLETED = 15u << 5,
    PCR0_NO_EVENT = 16u << 5
};

enum {
    PCR1_LOW_ORDER_BRANCH_ISSUED = 0u << 15,
    PCR1_PROCESSOR_CYCLE = 1u << 15,
    PCR1_DUAL_INSTRUCTION_ISSUE = 2u << 15,
    PCR1_BRANCH_MISPREDICTED = 3u << 15,
    PCR1_TLB_MISS = 4u << 15,
    PCR1_DTLB_MISS = 5u << 15,
    PCR1_DATA_CACHE_MISS = 6u << 15,
    PCR1_WBB_SINGLE_REQUEST_UNAVAILABLE = 7u << 15,
    PCR1_WBB_BURST_REQUEST_UNAVAILABLE = 8u << 15,
    PCR1_WBB_BURST_REQUEST_ALMOST_FULL = 9u << 15,
    PCR1_WBB_REQUEST_FULL = 10u << 15,
    PCR1_CPU_DATA_BUS_BUSY = 11u << 15,
    PCR1_INSTRUCTION_COMPLETED = 12u << 15,
    PCR1_NON_BDS_INSTRUCTION_COMPLETED = 13u << 15,
    PCR1_COP1_INSTRUCTION_COMPLETED = 14u << 15,
    PCR1_STORE_COMPLETED = 15u << 15,
    PCR1_NO_EVENT = 16u << 15
};

enum {
    PCR_U0 = 1u << 4,
    PCR_U1 = 1u << 14,
    PCR_CTE = 1u << 31
};

static inline void pcr_enable(u32 flags)
{
    flags |= (PCR_U0 | PCR_U1 | PCR_CTE);
    asm volatile(
        "mtps %[flags], 0\n\t"
        :
        : [flags] "r" (flags)
        : "memory"
    );
}

static inline void pcr_disable(void)
{
    asm volatile (
        "mtps $0, 0\n\t"
        :
        :
        : "memory"
    );
}

static inline u32 pcr_get0(void)
{
    u32 rv;
    asm (
        "mfpc %[rv], 0\n\t"
        : [rv] "=r" (rv)
    );

    return rv;
}

static inline u32 pcr_get1(void)
{
    u32 rv;
    asm (
        "mfpc %[rv], 1\n\t"
        : [rv] "=r" (rv)
    );

    return rv;
}

static inline void pcr_reset(void)
{
    asm volatile (
        "mtpc $0, 0\n\t"
        "mtpc $0, 1\n\t"
        ::: "memory"
    );
}

static inline u32 cpu_ticks(void) {
    u32 out;
    asm volatile("mfc0\t%0, $9\n" : "=r"(out) :: "memory");
    return out;
}

struct sprofile_counter {
	u64 time;
	u64 cycles;
	volatile u32 ticks;
    u64 pcr0, pcr1;
};

#define PROFILE_CNT(x) static struct sprofile_counter sprofile_##x

#define PROFILE_BEGIN(x) ({					                                    \
    pcr_reset();                                                                \
	sprofile_##x.ticks = cpu_ticks();		                                    \
    pcr_enable(PCR0_INSTRUCTION_CACHE_MISS | PCR1_DATA_CACHE_MISS);             \
})

#define PROFILE_END(x) ({		                                                \
    pcr_disable();                                                              \
	u32 start = sprofile_##x.ticks; 							                \
	volatile u32 end = cpu_ticks();												\
	u64 interval = end >= start ? end - start : (~((u32)0) - start) + end;		\
	sprofile_##x.time += interval;											    \
	sprofile_##x.cycles++;												        \
    sprofile_##x.pcr0 += pcr_get0();                                            \
    sprofile_##x.pcr1 += pcr_get1();                                            \
})

#endif //SPROFILE_H
