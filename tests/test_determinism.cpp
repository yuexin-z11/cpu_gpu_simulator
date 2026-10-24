#include "check.hpp"
#include "socsim/sweep_runner.hpp"

using namespace socsim;

// TODO: load sweeps/vecadd.json, run_sweep with 1 and 4 threads, and
// EXPECT every csv_row to be identical.
TEST(sweep_placeholder) {
    SweepSpec spec;
    EXPECT(run_sweep(spec, 4).empty());
}
