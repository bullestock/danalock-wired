#include <stdint.h>
#include <xtensa/xtruntime.h>

/// Disables interrupts and saves the interrupt enable flag in a register.
#define ACQ_LOCK()                                                             \
    uint32_t _pastlock;                                                        \
    __asm__ __volatile__ ("rsil %0, " XTSTR(XCHAL_EXCM_LEVEL) "\n"             \
                          : "=r"(_pastlock));

/// Restores the interrupte enable flag from a register.
#define REL_LOCK() __asm__ __volatile__ ("memw \n"                             \
                                         "wsr %0, ps\n"                        \
                                        :: "r"(_pastlock));

uint8_t __atomic_exchange_1(uint8_t *ptr, uint8_t val, int memorder)
{
    ACQ_LOCK();
    uint8_t ret = *ptr;
    *ptr = val;
    REL_LOCK();
    return ret;
}
