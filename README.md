# SoCSim: A CPU–GPU System Simulator

**Author:** Yuexin Zhang  
**Status:** Repository setup; the simulator is not implemented yet.

SoCSim is a planned cycle-counting simulator for a small chip with an RV32IM CPU, a GPU, and shared memory. Development has two phases. Phase 1 produces a deterministic C++ simulator and parallel experiment runner. Phase 2 adds an interactive interface and extends the hardware model. The Phase 2 tools will call the same simulator library.

## Phase 1: Core simulator

### Goal and model

The CPU runs a host program that prepares data and launches GPU work through a command queue. The GPU executes groups of threads called warps. Both processors have private L1 caches and share an L2 cache and DRAM. A run reports whether the program produced the correct result, its cycle count, and the main sources of delay.

The first version will model instruction cycle costs, cache hit and miss latency, DRAM latency and bandwidth, warp divergence, and memory request coalescing. Its cycle counts are intended for comparing configurations, not predicting a commercial chip.

### Inputs and outputs

- **Chip configuration:** JSON describing cache geometry, GPU warp count and size, and DRAM latency and bandwidth.
- **Program:** An RV32IM ELF binary containing CPU host code and a GPU kernel. Example source and precompiled ELF files are planned, so a RISC-V compiler is not needed to run the examples.
- **Sweep:** JSON describing a base configuration and parameter values to explore.
- **Single-run output:** PASS/FAIL, total cycles, CPU and GPU statistics, cache hit rates, divergence efficiency, and cache lines per warp request.
- **Sweep output:** One CSV row per configuration and a benchmark CSV with elapsed time at each worker count.

The intended command-line interface is:

```sh
./build/socsim run --config configs/default.json --program programs/bin/vecadd.elf
./build/socsim sweep --sweep sweeps/vecadd.json --threads 8 --out results/vecadd.csv
./build/socsim bench --sweep sweeps/vecadd.json --threads 1,2,4,8,16 --out results/speedup.csv
```

These commands are a design target; no executable or example ELF exists yet.

The planned Linux build is:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Phase 1 will document the tested GCC and CMake versions and the exact commands used to reproduce its reports.

### Architecture

```text
RV32IM CpuCore ── CPU L1 ──┐
                           ├── shared L2 ── Dram
GpuCore/Warps ── GPU L1 ───┘
       ▲
       │ command queue, doorbell, completion fence
       └──────────── CpuCore

Simulator: owns one chip and advances its clock
SweepRunner: runs independent Simulators on std::thread workers
```

The host writes data and a launch packet into memory, then rings a memory-mapped doorbell. The GPU reads the packet and executes the kernel. When it finishes, it writes a completion value to a fence address. The host waits for that value and checks the result.

### Planned components

| Component | Responsibility |
| --- | --- |
| `Cache`, `Dram`, `MemorySystem` | Model private L1 caches, shared L2, DRAM, and memory requests. |
| `ElfLoader`, `Decoder`, `CpuCore` | Load RV32IM ELF files and execute host instructions. |
| `GpuCore`, `Warp`, `Coalescer` | Schedule warps, handle branch divergence, and combine lane accesses into cache-line requests. |
| `CommandQueue` | Store and consume GPU launch packets in memory. |
| `Simulator`, `Stats` | Own one run, advance cycles, and collect structured results. |
| `SweepRunner` | Expand configurations and run independent simulations on `std::thread` workers. |

The simulation engine will be a C++17 library. The command-line program in `app/` will handle files and terminal output. Components will expose event hooks for Phase 2 replay recording. Each sweep worker will own its `Simulator`, and results will occupy stable output slots so thread count does not change simulation results.

### Example workloads and results to demonstrate

| Program | Purpose |
| --- | --- |
| `vecadd` | Basic launch and coalesced GPU memory access. |
| `vecadd_strided` | Same arithmetic with less efficient memory access. |
| `branchy` | Half of each warp follows a different branch. |
| `branchy_aligned` | Same work grouped to avoid warp divergence. |
| `matmul` | Matrix multiplication used to show L2 capacity effects. |

The planned comparisons are: strided access uses more cache lines and cycles than coalesced access; branch divergence reduces active-lane efficiency; a larger L2 reduces misses on a suitable workload. A sweep of at least 64 configurations will be timed with 1, 2, 4, 8, and 16 workers, then compared for identical per-configuration results.

### Phase 1 checklist

- [ ] Add CMake targets for the simulator library and `socsim` executable.
- [ ] Define configuration, memory request, event, and statistics types.
- [ ] Implement cache, DRAM, memory routing, and configuration parsing.
- [ ] Implement ELF loading and RV32IM CPU decoding and execution.
- [ ] Implement command launches, GPU warps, divergence, and coalescing.
- [ ] Add example sources and precompiled ELF binaries.
- [ ] Add instruction, cache, coalescer, end-to-end, and determinism checks.
- [ ] Add the parallel sweep runner, CSV output, and plotting script.
- [ ] Record comparison results, a speedup graph, build instructions, and a demo video.

## Phase 2: Interactive exploration and extensions

Phase 2 begins after the core simulator works. Its first milestone is a local website that can rerun and replay simulations.

### 2.1 Local interactive website

Start a local C++ server with `./build/socsim serve --port 8080` and open `http://localhost:8080`. The server will host the website, call the Phase 1 simulator library, and bind to localhost. It will validate configuration and program inputs and use background jobs for long sweeps.

Planned endpoints include `GET /api/programs`, `GET /api/config/default`, `POST /api/run`, `POST /api/sweep`, and `GET /api/jobs/:id`. The run panel will let users edit chip settings, choose a workload, and choose a replay window. The explorer will run sweeps, chart cycle and stall breakdowns, and compare pinned configurations. The replay will animate CPU phases, GPU warp lanes, command launches, fences, and cache activity. An advisor will rank findings such as poor coalescing, divergence, and memory stalls and link them to replay events.

### 2.2 More realistic hardware

Add a pipelined CPU, hazards and branch prediction, instruction fetch and TLB effects, banked DRAM and request scheduling, MSHRs and cache contention, GPU occupancy and shared memory, barriers and scoreboarding, atomics, multiple GPU cores, and more realistic CPU–GPU synchronization and transfer costs.

### 2.3 Deeper analysis

Add a full CPI and warp stall breakdown, source-line mapping through DWARF, more advisor rules, and automated what-if runs that estimate cycles saved by proposed changes.

### 2.4 In-browser version

Compile the simulator library to WebAssembly and run it in a Web Worker. Add a built-in RISC-V assembler, editable kernels, optimization challenges, and pipeline animation. Host the static interface on GitHub Pages while retaining the native server for large sweeps.

### 2.5 Validation and longer-term extensions

Compare microbenchmarks with real hardware, compare CPU execution with Spike, and calibrate parameters from hardware documentation. Possible later work includes an out-of-order CPU, multicore cache coherence, and a SystemVerilog GPU model compared cycle by cycle with the C++ simulator.

## Repository layout

```text
cpu_gpu_simulator/
├── README.md              # Two-phase plan and current status
├── app/                   # Phase 1 CLI entry point
├── configs/               # Chip configuration files
├── docs/                  # Design notes and demo materials
├── include/socsim/        # Public C++ library headers
├── phase2/
│   ├── server/             # Local HTTP server
│   └── web/                # Browser interface
├── programs/
│   ├── src/                # Example program source
│   └── bin/                # Precompiled RV32IM ELF files
├── results/               # Reports, CSV files, and plots
├── src/                   # Phase 1 simulator implementation
├── sweeps/                # Experiment definitions
├── tests/                 # Component and integration checks
└── tools/                 # Plotting and helper scripts
```

Folders currently contain placeholders. Add subsystem folders under `src/` and website asset folders under `phase2/web/` when code needs them.

## Planned tools and dependencies

Phase 1 targets Linux, C++17, CMake, and the C++ standard library. Python 3 with matplotlib will generate plots. A RISC-V GCC toolchain will be needed only to rebuild example ELF files. The Phase 2 local server is planned to use cpp-httplib and nlohmann/json; the in-browser stage will use Emscripten.
