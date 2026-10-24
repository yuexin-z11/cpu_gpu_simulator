#pragma once
// One function per window panel. Each only reads from the Replay, except
// draw_controls, which may start a new run.

#include <string>

#include "replay.hpp"

namespace gui {

struct UiState {
    socsim::ChipConfig config;
    std::string program;
    std::string error;
};

void draw_controls(UiState& ui, Replay& replay);  // config, program, run/step/play
void draw_timeline(const Replay& replay);         // CPU setup / launch / kernel / fence
void draw_warp_lanes(const Replay& replay);       // active lanes per warp at cursor
void draw_cache(const Replay& replay);            // L1/L2 hits and misses over time
void draw_stats(const Replay& replay);            // PASS, cycles, hit rates, metrics

}  // namespace gui
