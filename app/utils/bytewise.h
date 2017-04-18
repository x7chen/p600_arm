#ifndef BYTEWISE_H_
#define BYTEWISE_H_
#include <stdint.h>

#define Tranverse16(X)                 ((((uint16_t)(X) & 0xff00) >> 8) |(((uint16_t)(X) & 0x00ff) << 8))
#define Tranverse32(X)                 ((((uint32_t)(X) & 0xff000000) >> 24) | \
                                        (((uint32_t)(X) & 0x00ff0000) >> 8) | \
                                        (((uint32_t)(X) & 0x0000ff00) << 8) | \
                                        (((uint32_t)(X) & 0x000000ff) << 24))
#define Tranverse64(X)                 ((((uint64_t)(X) & 0xff00000000000000) >> 56) | \
                                        (((uint64_t)(X) & 0x00ff000000000000) >> 40) | \
                                        (((uint64_t)(X) & 0x0000ff0000000000) << 24) | \
                                        (((uint64_t)(X) & 0x000000ff00000000) << 8) | \
                                        (((uint64_t)(X) & 0x000000000ff00000) >> 8) | \
                                        (((uint64_t)(X) & 0x0000000000ff0000) >> 24) | \
                                        (((uint64_t)(X) & 0x000000000000ff00) << 40) | \
                                        (((uint64_t)(X) & 0x00000000000000ff) << 56))
uint32_t swap_bits(uint32_t inp);
uint32_t bytewise_bitswap(uint32_t inp);
#endif
