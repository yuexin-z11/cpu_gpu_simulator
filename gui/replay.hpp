#pragma once
// Runs one simulation with event recording on and steps through the events.

#include <cstddef>
#include <string>

#include "socsim/config.hpp"
#include "socsim/event_log.hpp"
#include "socsim/stats.hpp"

namespace gui {

class Replay {
public:
    // Runs the program to completion with the event log enabled.
    void load(const socsim::ChipConfig& cfg, const std::string& program);

    bool loaded() const { return loaded_; }
    const socsim::RunResult& result() const { return result_; }
    const socsim::EventLog& log() const { return log_; }
    const socsim::ChipConfig& config() const { return cfg_; }

    // Playback position, in simulated cycles.
    socsim::Cycle cursor() const { return cursor_; }
    void step(socsim::Cycle cycles);
    void seek(socsim::Cycle cycle);

    bool playing = false;
    float cycles_per_second = 200.0f;

private:
    socsim::ChipConfig cfg_;
    socsim::EventLog log_;
    socsim::RunResult result_;
    socsim::Cycle cursor_ = 0;
    bool loaded_ = false;
};

}  // namespace gui
