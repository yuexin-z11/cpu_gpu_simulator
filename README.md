# SoCSim: A CPU–GPU System Simulator

**Author:** Isaac Geng  
**Status:** Project structure started; implementation is still to do.

SoCSim is a cycle-counting simulator for a small system with an RV32IM CPU and a GPU sharing a memory hierarchy. The project is planned in two phases. Phase 1 is the course final project; Phase 2 is a personal continuation built on the Phase 1 simulator library.

## Phase 1: Course Final Project

### Goal

Build a working C++17 simulator that runs a CPU host program, launches GPU work, and reports correctness and cycle-level performance statistics. The special topic is multithreading with `std::thread`: independent simulator runs are distributed across workers by a parallel sweep runner.

### Scope

- Model configurable CPU and GPU L1 caches, a shared L2 cache, and DRAM.
- Load RV32IM ELF programs, decode instructions, and execute host and GPU code.
- Run GPU threads in warps with a round-robin scheduler and model branch divergence.
- Coalesce warp memory accesses into cache-line requests.
- Collect structured counters for cycles, cache behavior, stalls, divergence, and coalescing.
- Run parameter sweeps in parallel and save result and timing CSV files.
- Include example programs for vector addition, strided accesses, branching, and matrix multiplication.

### Required demonstrations

1. Example programs pass their own result checks; CPU instructions are covered by tests.
2. Strided accesses use more cache lines and cycles than coalesced accesses; divergent branches show lower efficiency; increasing L2 capacity reduces misses on a suitable workload.
3. A sweep of at least 64 configurations is timed at 1, 2, 4, 8, and 16 worker threads and graphed.
4. Sweep results are identical for every worker count.

### Build and run (planned)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/socsim run --config configs/default.json --program programs/bin/vecadd.elf
./build/socsim sweep --sweep sweeps/vecadd.json --threads 8 --out results/vecadd.csv
./build/socsim bench --sweep sweeps/vecadd.json --threads 1,2,4,8,16 --out results/speedup.csv
```

These commands describe the intended interface; the executable and build files have not been implemented yet.

### Planned Phase 1 components

`Cache`, `Dram`, `MemorySystem`, `ElfLoader`, `Decoder`, `CpuCore`, `GpuCore`, `Warp`, `Coalescer`, `CommandQueue`, `Simulator`, `Stats`, and `SweepRunner`. The simulator library should contain no terminal or file I/O; `app/` will provide the command-line wrapper.

### Phase 1 deliverables

- C++17 source, CMake build, configs, sweeps, and example program source and binaries.
- `README.md` instructions to reproduce the results.
- Test coverage for instruction behavior, cache behavior, coalescing, end-to-end programs, and deterministic parallel sweeps.
- CSV results, plots, and a video showing the build and required demonstrations.

## Phase 2: Personal Continuation

Phase 2 extends the Phase 1 library. Its first milestone is a local interactive website for configuring and replaying simulations.

### 2.1 Local interactive website

Run `./build/socsim serve --port 8080`, then open `http://localhost:8080`. A small C++ HTTP server will serve the website and call the same simulator library as the command-line tool. Planned API operations list programs, return the default config, run simulations and sweeps, and report progress for background jobs. The server should bind to localhost and validate inputs.

The website will provide a run panel, editable chip and program settings, sweep explorer, charts, run comparison, and an animated replay showing CPU progress, GPU warps and lanes, command launches, and cache events. Event hooks added to Phase 1 components will supply replay data. An advisor will later turn statistics into ranked optimization suggestions.

### 2.2 More realistic hardware

Planned extensions include a pipelined CPU with hazards and branch prediction; TLBs; banked DRAM and memory scheduling; MSHRs and cache arbitration; GPU occupancy, shared memory, barriers, scoreboarding, atomics, and multiple cores; and more realistic CPU–GPU transfer and synchronization costs.

### 2.3 Deeper analysis

Add a fuller CPI and warp stall breakdown, more advisor rules, source-line mapping through DWARF, and what-if runs that estimate cycles saved by configuration changes.

### 2.4 In-browser version

Compile the simulator library to WebAssembly and run it in a Web Worker. Add a browser assembler, interactive optimization challenges, and pipeline animation. Host the static website on GitHub Pages; retain the native server for larger parallel sweeps.

### 2.5 Validation and extensions

Validate microbenchmarks against real hardware, compare CPU execution against Spike, and use published hardware data to guide parameters. Longer-term extensions include out-of-order and multicore CPUs, cache coherence, and a SystemVerilog GPU model checked against the C++ simulator.

## Repository layout

```text
personal_project/
├── app/                  # TODO: command-line entry point
├── configs/              # Chip configuration JSON files
├── docs/                 # Project notes and submission materials
├── include/socsim/       # TODO: public simulator headers
├── phase2/
│   ├── server/            # TODO: local HTTP server
│   └── web/               # TODO: website source and static assets
├── programs/
│   ├── src/               # Example host and kernel sources
│   └── bin/               # Precompiled example ELF files
├── results/               # Generated reports, CSV files, and plots
├── src/                   # TODO: simulator implementation by subsystem
├── sweeps/                # Sweep definitions
├── tests/                 # TODO: unit and end-to-end tests
└── tools/                 # TODO: result plotting and helper scripts
```

## Implementation checklist

- [ ] Add a CMake project and a `socsim` library plus CLI target.
- [ ] Define shared config, event, and structured statistics types.
- [ ] Implement cache, DRAM, memory system, and configuration parsing.
- [ ] Implement ELF loading, RV32IM decoding, CPU execution, and GPU warps.
- [ ] Add command queue, launch/fence behavior, divergence, and coalescing.
- [ ] Implement deterministic simulator stepping and example programs.
- [ ] Add unit and end-to-end tests and remove compiler warnings.
- [ ] Implement the `std::thread` sweep runner and benchmark output.
- [ ] Generate required reports and plots and record the submission video.
- [ ] After Phase 1, implement Phase 2 milestones in order.

## Dependencies

The planned Phase 1 simulator uses only the C++ standard library and CMake. Plotting uses Python 3 and matplotlib. Rebuilding example ELFs requires a RISC-V GCC toolchain; precompiled binaries are intended to be included so running examples does not require that toolchain. Phase 2 plans to use cpp-httplib and nlohmann/json for the local server.
