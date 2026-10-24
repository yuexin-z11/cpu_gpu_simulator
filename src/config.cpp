#include "socsim/config.hpp"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace socsim {

ChipConfig config_from_json(const nlohmann::json& j) {
    ChipConfig c;
    // TODO: read each section ("cpu", "gpu", "cpu_l1", "gpu_l1", "l2",
    // "dram", "max_cycles"), keeping the defaults for missing keys.
    (void)j;
    return c;
}

nlohmann::json config_to_json(const ChipConfig& c) {
    // TODO: inverse of config_from_json.
    (void)c;
    return nlohmann::json::object();
}

ChipConfig load_config(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open config: " + path);
    ChipConfig c = config_from_json(nlohmann::json::parse(in));
    validate(c);
    return c;
}

void validate(const ChipConfig& c) {
    // TODO: check cache geometry (power-of-two lines, size divisible by
    // line_bytes * ways), nonzero warp size <= 32, nonzero DRAM bandwidth.
    (void)c;
}

}  // namespace socsim
