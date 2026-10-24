# SoCSim: A CPU–GPU System Simulator

**Author:** Yuexin Zhang  
**Status:** Proposal approved. The core library, CLI, and placeholder tests build. GUI and CUDA skeletons are present but have not been verified. Simulator components still contain `TODO` stubs.

SoCSim is a cycle-counting simulator for a small chip with an RV32IM CPU, a SIMT GPU, and shared memory. It is a deterministic C++17 library with a command-line program, a parallel sweep runner, and a desktop window that replays recorded runs. Its cycle counts are intended for comparing configurations, not predicting a commercial chip.

Work is split into three tiers so complete results exist even if later parts run long:

| Tier | Contents | Done when |
| --- | --- | --- |
| **Must-have** | CPU, GPU with divergence and coalescing, caches, DRAM, one-packet launch, sweep runner, the five workloads, the three comparisons, the speedup benchmark | All measurable results below are recorded |
| **Should-have** | Desktop replay window (GLFW + OpenGL + Dear ImGui); CUDA trend check on a real NVIDIA GPU | Window replays every example workload; CUDA results recorded next to SoCSim's |
| **Stretch** | Items under [Stretch goals](#stretch-goals) | Only started after the must-have results are recorded |

## Must-have: core simulator

### Model

The CPU runs a host program that prepares data and launches one GPU kernel. The GPU executes groups of threads called warps. Each processor has a private L1 cache, and both share an L2 cache and DRAM. A run reports whether the program produced the correct result, its cycle count, and the main sources of delay.

The must-have model deliberately keeps these simplifications:

- **CPU:** in-order, one instruction at a time, fixed cycle cost per instruction class plus memory latency. No pipeline or branch prediction.
- **Caches model timing only.** All data lives in one flat functional memory, and the caches track tags to decide hit or miss latency. This removes the need for cache coherence between the CPU and GPU L1 caches.
- **DRAM:** fixed latency plus a bandwidth limit in bytes per cycle. No banks or request scheduling.
- **Launch:** a single launch packet at a fixed address, a memory-mapped doorbell, and a completion fence. No multi-entry queue.
- **GPU:** one GPU core with a configurable number of warps and warp size, round-robin warp scheduling, one instruction per warp per issue.

If ELF loading or the RISC-V example programs run long, the GPU comparisons fall back to small hand-assembled kernels loaded directly from test code, so the coalescing and divergence results do not depend on the toolchain path.

### GPU instruction set

The GPU executes the **same RV32IM instruction set as the CPU**, in SIMT style: every lane of a warp runs the same kernel code on its own registers.

- **Thread identity.** A kernel reads its lane ID, warp ID, and block size from read-only CSRs in the RISC-V custom user range (`0xCC0`–`0xCC2`) using the standard `csrr` instruction. The kernel arguments pointer is passed in `a0`. This needs no custom compiler support.
- **Divergence.** Each lane has its own program counter. At each issue the warp executes the instruction at the lowest PC among its unfinished lanes, and only lanes at that PC are active. Lanes reconverge automatically when their PCs meet again after an `if`/`else` or a loop. Divergence efficiency is active lanes divided by warp size, averaged over issued instructions.
- **Coalescing.** For each load or store, the coalescer groups the active lanes' addresses into distinct cache-line requests. The reported metric is cache lines per warp memory instruction.
- **Unsupported on the GPU:** `ecall`, `ebreak`, `fence`, and CSRs other than the three above. A kernel ends when every lane executes `ret` from the kernel entry point.

The CPU host code and the GPU kernel are compiled into one RV32IM ELF file. The launch packet holds the kernel entry address, the argument pointer, and the number of warps.

### Inputs and outputs

- **Chip configuration:** JSON describing cache geometry, GPU warp count and size, and DRAM latency and bandwidth.
- **Program:** an RV32IM ELF binary. Example sources are present, but precompiled ELF files are not yet available. A RISC-V compiler will be needed to build them once the programs are implemented.
- **Sweep:** JSON describing a base configuration and parameter values to explore.
- **Single-run output:** PASS/FAIL, total cycles, CPU and GPU statistics, cache hit rates, divergence efficiency, and cache lines per warp memory instruction.
- **Sweep output:** one CSV row per configuration, and a benchmark CSV with elapsed time at each worker count.

```sh
./build/socsim run   --config configs/default.json --program programs/bin/vecadd.elf
./build/socsim sweep --sweep sweeps/vecadd.json --threads 8 --out results/vecadd.csv
./build/socsim bench --sweep sweeps/vecadd.json --threads 1,2,4,8,16 --out results/speedup.csv
```

The current core skeleton builds on Linux with:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/socsim_tests
```

nlohmann/json is downloaded automatically if it is not installed. The commands above currently verify only the skeleton; the run, sweep, and bench examples require unfinished components and ELF programs. The report will list the tested GCC and CMake versions and the exact commands used to reproduce each result.

### Architecture

```text
RV32IM CpuCore ── CPU L1 ──┐
                           ├── shared L2 ── Dram      (timing)
GpuCore/Warps ── GPU L1 ───┘
       ▲                        FlatMemory            (data)
       │ launch packet, doorbell, completion fence
       └──────────── CpuCore

Simulator:   owns one chip and advances its clock
SweepRunner: runs independent Simulators on std::thread workers
```

The host writes input data and the launch packet into memory, then writes the doorbell address. The GPU reads the packet and runs the kernel. When every warp finishes, the GPU writes a completion value to the fence address. The host polls the fence and then checks the result.

### Components

| Component | Responsibility |
| --- | --- |
| `FlatMemory`, `Cache`, `Dram`, `MemorySystem` | Hold program data; model L1, shared L2, and DRAM timing. |
| `ElfLoader`, `Decoder`, `Executor` | Load static RV32IM ELF files; decode and execute instructions for both processors. |
| `CpuCore` | Run host code and handle the doorbell and fence addresses. |
| `GpuCore`, `Warp`, `Coalescer` | Schedule warps, select active lanes by lowest PC, and combine lane accesses into cache-line requests. |
| `Simulator`, `Stats`, `EventLog` | Own one run, advance cycles, collect results, and optionally record events for replay. |
| `SweepRunner` | Expand configurations and run independent simulations on `std::thread` workers. |

The CPU and GPU share one decoder and executor, which keeps the instruction set implementation in one place. Each sweep worker owns its `Simulator`, and results go into fixed output slots, so the thread count cannot change any result.

### Workloads

| Program | Purpose |
| --- | --- |
| `vecadd` | Basic launch and coalesced GPU memory access. |
| `vecadd_strided` | Same arithmetic with strided, poorly coalesced access. |
| `branchy` | Half of each warp follows a different branch. |
| `branchy_aligned` | Same work grouped by warp so no warp diverges. |
| `matmul` | Small matrix multiplication whose working set exceeds small L2 sizes. |

### Measurable results

1. **Coalescing:** `vecadd_strided` uses more cache lines per warp memory instruction and more cycles than `vecadd`.
2. **Divergence:** `branchy` has about 50% divergence efficiency; `branchy_aligned` stays near 100% and finishes in fewer cycles.
3. **L2 size:** on `matmul`, L2 misses and total cycles fall as L2 size grows until the working set fits.
4. **Parallel sweep:** a sweep of at least 64 configurations is timed with 1, 2, 4, 8, and 16 workers and plotted as a speedup graph.
5. **Determinism:** every per-configuration result is identical across all worker counts.

### Must-have checklist

- [ ] CMake targets for the simulator library and the `socsim` executable.
- [ ] Configuration, memory request, statistics, and event types.
- [ ] Flat memory, caches, DRAM, memory routing, and configuration parsing.
- [ ] ELF loading and the shared RV32IM decoder and executor.
- [ ] CPU core with the doorbell and fence addresses.
- [ ] GPU core with lane CSRs, lowest-PC divergence, and the coalescer.
- [ ] The five example sources and precompiled ELF binaries.
- [ ] Instruction, cache, coalescer, end-to-end, and determinism checks.
- [ ] Parallel sweep runner, CSV output, and plotting script.
- [ ] The five measurable results, build instructions, and a demo video.

## Should-have: desktop replay window

After the must-have results are recorded, add `socsim-gui`, a desktop window built with GLFW, OpenGL, Dear ImGui, and ImPlot. It reuses the dependency approach from my ECE 6122 Lab 3 build: search the active conda environment first, fetch missing libraries with CMake FetchContent, and load OpenGL functions through GLFW instead of linking libGL on Linux.

```sh
./build/socsim-gui                                  # choose config and program in the window
./build/socsim-gui --program programs/bin/branchy.elf
```

The window calls the same simulator library, records an event log for the run, and replays it. It never changes simulation results.

```text
┌─ SoCSim ───────────────────────────────────────────────────────────┐
│ Config         │ Timeline: CPU setup │ launch │ GPU kernel │ fence │
│  L1 size  [▾]  ├────────────────────────────────────────────────────┤
│  L2 size  [▾]  │ Warp lanes (filled = active, empty = masked off)   │
│  warps    [4]  │  W0 ■■■■■■■■□□□□□□□□   W1 ■■■■■■■■■■■■■■■■         │
│  DRAM lat [..] ├────────────────────────────────────────────────────┤
│ Program [▾]    │ Cache: L1/L2 hits and misses over time             │
│ [Run] [Step]   ├────────────────────────────────────────────────────┤
│ [Play] [Speed] │ PASS  cycles  L2 hit rate  lines/req  div. eff.    │
└────────────────┴────────────────────────────────────────────────────┘
```

The window is a separate CMake target behind `-DSOCSIM_BUILD_GUI=ON`, so the CLI and sweeps still build and run on machines without a display. The window is built, run, and verified only on PACE-ICE, from a terminal opened inside an OnDemand Interactive Desktop session (a separate browser VS Code terminal does not share that display). The demo video will use it to show the coalescing and divergence comparisons side by side.

- [ ] `socsim-gui` target with GLFW, OpenGL, Dear ImGui, and ImPlot.
- [ ] Configuration and program panels that start a run.
- [ ] Timeline, warp-lane, cache, and statistics panels driven by the event log.
- [ ] Step, play, and speed controls.
- [ ] Window verified on a PACE-ICE OnDemand Interactive Desktop.

## Should-have: CUDA trend check

To check that SoCSim's trends are realistic, the four GPU comparison workloads are also written as CUDA kernels and run on real NVIDIA GPUs in two places: a local RTX 3080 Ti Laptop GPU (compute capability 8.6, CUDA 12.9 under WSL2) and a PACE-ICE GPU node. The report lists the GPU model, driver, and CUDA version for each machine, and the trends must hold on both. The comparison is about direction, not cycle counts: the strided kernel should be slower than the coalesced one, and the divergent kernel slower than the aligned one.

| CUDA kernel | Matches SoCSim workload |
| --- | --- |
| `vecadd` | `vecadd` |
| `vecadd_strided` | `vecadd_strided` |
| `branchy` | `branchy` |
| `branchy_aligned` | `branchy_aligned` |

Each kernel is timed with `cudaEvent` over repeated launches after a warm-up, checked against a CPU reference, and written to one CSV per machine (`results/cuda_trends_local.csv` and `results/cuda_trends_pace.csv`). The report places these timings next to SoCSim's cycle counts and metrics for the same pairs.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSOCSIM_BUILD_CUDA=ON
cmake --build build -j
./build/socsim-cuda --out results/cuda_trends_local.csv   # or cuda_trends_pace.csv on PACE-ICE
```

`socsim-cuda` is a separate target behind `-DSOCSIM_BUILD_CUDA=ON`, so the simulator, sweeps, and window build without a GPU or the CUDA Toolkit. It never links into or changes the simulator library.

- [ ] `socsim-cuda` target with the four kernels and CPU result checks.
- [ ] Timing harness and CSV output.
- [ ] Runs recorded on both the local GPU and a PACE-ICE GPU node.
- [ ] Side-by-side table of CUDA and SoCSim trends in the report.

## Stretch goals

These start only after the must-have results are recorded, roughly in this order:

1. Multi-entry command queue and several kernel launches per program.
2. Banked DRAM with request scheduling, and MSHRs for overlapping misses.
3. Pipelined CPU with hazards and branch prediction.
4. GPU shared memory, barriers, atomics, and multiple GPU cores.
5. CPI and warp-stall breakdown, and an advisor that flags poor coalescing, divergence, and memory stalls.
6. Extend the CUDA trend check with `matmul` and Nsight Compute profiling, comparing memory sectors per request and branch efficiency with SoCSim's coalescing and divergence metrics.
7. Local web interface or a WebAssembly build of the library.

## Originality

SoCSim is new work written for this course. It is not based on an assignment or simulator from another course, and it does not copy code from existing simulators. Published tools such as Spike (RISC-V ISA simulator), GPGPU-Sim, and Vortex (a RISC-V GPGPU) are background reading only. Their ideas will be cited in the report where they influenced a design choice. The only reused material is the build approach from my own ECE 6122 Lab 3 for the optional window. The CUDA kernels are written new for this comparison. Third-party libraries (GLFW, Dear ImGui, ImPlot, nlohmann/json) are used unmodified under their licenses.

## Repository layout

```text
cpu_gpu_simulator/
├── CMakeLists.txt         # Library, CLI, tests; GUI and CUDA options
├── README.md              # Plan, scope tiers, and current status
├── app/main.cpp           # CLI entry point: run, sweep, bench
├── configs/default.json   # Default chip configuration
├── cuda/                  # Should-have CUDA trend check (trends.cu)
├── docs/                  # Design notes, report, and demo materials
├── gui/                   # Should-have replay window (main, replay, panels)
├── include/socsim/        # Public headers, one per component
├── phase2/                # Stretch: local server and web interface
│   ├── server/
│   └── web/
├── programs/
│   ├── Makefile           # Rebuilds bin/*.elf with a RISC-V GCC
│   ├── src/               # Five workloads; common/ has crt0, linker script, socsim.h
│   └── bin/               # Planned RV32IM ELF files (currently empty)
├── results/               # Reports, CSV files, and plots
├── src/                   # Component implementations
├── sweeps/                # Experiment definitions (vecadd, matmul_l2)
├── tests/                 # Checks using a small built-in framework (check.hpp)
└── tools/                 # plot_sweep.py, plot_speedup.py
```

## Tools and dependencies

The core targets Linux, C++17, CMake, and the C++ standard library, plus nlohmann/json for configuration files. Python 3 with matplotlib generates plots. A RISC-V GCC toolchain is needed to build the example ELF files. The window adds GLFW, OpenGL 3.3, Dear ImGui, and ImPlot, and is run on PACE-ICE. The CUDA trend check adds the CUDA Toolkit (12.9 from the `ece4122-6122` environment) and an NVIDIA GPU; locally on WSL2 it uses the Windows NVIDIA driver, with no Linux driver installed inside WSL, and on PACE-ICE it runs on an allocated GPU node.
