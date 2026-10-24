// socsim-cuda: times the four comparison workloads on a real GPU and writes
// one CSV row per kernel. Only the direction of each comparison matters:
// strided slower than coalesced, divergent slower than aligned.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <cuda_runtime.h>

#define CUDA_CHECK(call)                                                    \
    do {                                                                    \
        cudaError_t err_ = (call);                                          \
        if (err_ != cudaSuccess) {                                          \
            std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__,         \
                         cudaGetErrorString(err_));                         \
            std::exit(1);                                                   \
        }                                                                   \
    } while (0)

// ------------------------------------------------------------ kernels -----

__global__ void vecadd(const int* a, const int* b, int* c, int n) {
    // TODO: c[i] = a[i] + b[i] with i = global thread index.
    (void)a; (void)b; (void)c; (void)n;
}

__global__ void vecadd_strided(const int* a, const int* b, int* c, int n, int stride) {
    // TODO: same arithmetic, but thread i touches element (i * stride) % n.
    (void)a; (void)b; (void)c; (void)n; (void)stride;
}

__global__ void branchy(const int* in, int* out, int n) {
    // TODO: even and odd threads take different, equally long branches.
    (void)in; (void)out; (void)n;
}

__global__ void branchy_aligned(const int* in, int* out, int n) {
    // TODO: same work, but choose the branch by (threadIdx.x / warpSize)
    // so no warp diverges.
    (void)in; (void)out; (void)n;
}

// ------------------------------------------------------------ harness -----

struct Row {
    std::string kernel;
    double ms_per_launch = 0.0;
    bool correct = false;
};

// Times `launch` over `reps` launches after one warm-up, using cudaEvent.
template <typename Launch>
double time_ms(Launch launch, int reps) {
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    launch();  // warm-up
    CUDA_CHECK(cudaEventRecord(start));
    for (int i = 0; i < reps; ++i) launch();
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    return ms / reps;
}

int main(int argc, char** argv) {
    std::string out_path = "results/cuda_trends.csv";
    for (int i = 1; i + 1 < argc; ++i)
        if (std::strcmp(argv[i], "--out") == 0) out_path = argv[++i];

    cudaDeviceProp prop{};
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    int runtime = 0, driver = 0;
    CUDA_CHECK(cudaRuntimeGetVersion(&runtime));
    CUDA_CHECK(cudaDriverGetVersion(&driver));
    std::printf("GPU: %s (sm_%d%d), runtime %d, driver %d\n",
                prop.name, prop.major, prop.minor, runtime, driver);

    std::vector<Row> rows;
    // TODO:
    //  1. Allocate and fill inputs on the host; copy to the device.
    //  2. For each kernel, time_ms(...) with a fixed grid/block size, copy
    //     the output back, and compare with a CPU reference.
    //  3. Push one Row per kernel.

    std::ofstream out(out_path);
    if (!out) {
        std::fprintf(stderr, "error: cannot write %s\n", out_path.c_str());
        return 1;
    }
    out << "gpu,sm,kernel,ms_per_launch,correct\n";
    for (const Row& r : rows)
        out << prop.name << ",sm_" << prop.major << prop.minor << ',' << r.kernel << ','
            << r.ms_per_launch << ',' << (r.correct ? 1 : 0) << '\n';
    return 0;
}
