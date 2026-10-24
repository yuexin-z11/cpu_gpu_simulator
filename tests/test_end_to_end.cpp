#include "check.hpp"
#include "socsim/config.hpp"
#include "socsim/simulator.hpp"

using namespace socsim;

TEST(default_config_loads) {
    const ChipConfig c = load_config(SOCSIM_SOURCE_DIR "/configs/default.json");
    EXPECT(c.gpu.warp_size > 0);
}

// TODO: for each programs/bin/*.elf, run it on configs/default.json and
// EXPECT(result.passed). Then check the measurable-result trends:
//  - vecadd_strided has more lines per mem instruction than vecadd
//  - branchy ~0.5 divergence efficiency, branchy_aligned ~1.0
