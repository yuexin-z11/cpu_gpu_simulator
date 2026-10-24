/* branchy: Even and odd lanes take different branches, so every warp diverges. */
#include "common/socsim.h"

#define N 1024

typedef struct {
    /* TODO: pointers and sizes the kernel needs. */
    uint32_t n;
} args_t;

/* GPU kernel: runs once per lane. */
void kernel(void* p) {
    args_t* args = (args_t*)p;
    /* TODO: global index = warp_id() * block_size() + lane_id(); do the work. */
    (void)args;
}

int main(void) {
    static args_t args;
    /* TODO: initialise inputs. */
    args.n = N;
    launch_kernel(kernel, &args, 4);
    /* TODO: check outputs; return 0 on success, 1 on failure. */
    return 0;
}
