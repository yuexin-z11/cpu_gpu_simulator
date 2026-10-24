/* Shared definitions for SoCSim example programs.
 * Addresses must match include/socsim/types.hpp. */
#ifndef SOCSIM_H
#define SOCSIM_H

#include <stdint.h>

#define SOCSIM_LAUNCH_PACKET 0x00010000u
#define SOCSIM_DOORBELL      0x00010100u
#define SOCSIM_FENCE         0x00010104u
#define SOCSIM_FENCE_DONE    1u

typedef struct {
    uint32_t kernel_entry;
    uint32_t args_ptr;
    uint32_t num_warps;
} launch_packet_t;

/* GPU lane identity (read-only CSRs 0xCC0-0xCC2). */
static inline uint32_t lane_id(void)    { uint32_t v; __asm__ volatile("csrr %0, 0xCC0" : "=r"(v)); return v; }
static inline uint32_t warp_id(void)    { uint32_t v; __asm__ volatile("csrr %0, 0xCC1" : "=r"(v)); return v; }
static inline uint32_t block_size(void) { uint32_t v; __asm__ volatile("csrr %0, 0xCC2" : "=r"(v)); return v; }

/* Host side: write the packet, ring the doorbell, wait for the fence. */
static inline void launch_kernel(void (*kernel)(void*), void* args, uint32_t num_warps) {
    volatile launch_packet_t* pkt = (volatile launch_packet_t*)SOCSIM_LAUNCH_PACKET;
    volatile uint32_t* doorbell = (volatile uint32_t*)SOCSIM_DOORBELL;
    volatile uint32_t* fence = (volatile uint32_t*)SOCSIM_FENCE;
    *fence = 0;
    pkt->kernel_entry = (uint32_t)(uintptr_t)kernel;
    pkt->args_ptr = (uint32_t)(uintptr_t)args;
    pkt->num_warps = num_warps;
    *doorbell = 1;
    while (*fence != SOCSIM_FENCE_DONE) {
    }
}

#endif
