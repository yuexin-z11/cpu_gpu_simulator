#include "replay.hpp"

#include <algorithm>

#include "socsim/simulator.hpp"

namespace gui {

void Replay::load(const socsim::ChipConfig& cfg, const std::string& program) {
    cfg_ = cfg;
    socsim::Simulator sim(cfg);
    sim.event_log().set_enabled(true);
    sim.load_program(program);
    result_ = sim.run();
    log_ = sim.event_log();
    cursor_ = 0;
    loaded_ = true;
}

void Replay::step(socsim::Cycle cycles) {
    seek(cursor_ + cycles);
}

void Replay::seek(socsim::Cycle cycle) {
    cursor_ = std::min(cycle, result_.total_cycles);
}

}  // namespace gui
