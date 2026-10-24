#include "panels.hpp"

#include <exception>

#include <imgui.h>
#include <implot.h>

namespace gui {

void draw_controls(UiState& ui, Replay& replay) {
    ImGui::Begin("Config");
    // TODO: widgets for cache sizes, warp count/size, DRAM latency, and a
    // program picker listing programs/bin/*.elf.
    ImGui::Text("program: %s", ui.program.empty() ? "(none)" : ui.program.c_str());
    if (ImGui::Button("Run") && !ui.program.empty()) {
        try {
            replay.load(ui.config, ui.program);
            ui.error.clear();
        } catch (const std::exception& e) {
            ui.error = e.what();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Step")) replay.step(1);
    ImGui::SameLine();
    if (ImGui::Button(replay.playing ? "Pause" : "Play")) replay.playing = !replay.playing;
    ImGui::SliderFloat("cycles/s", &replay.cycles_per_second, 1.0f, 100000.0f, "%.0f",
                       ImGuiSliderFlags_Logarithmic);
    if (!ui.error.empty()) ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", ui.error.c_str());
    ImGui::End();
}

void draw_timeline(const Replay& replay) {
    ImGui::Begin("Timeline");
    // TODO: draw CpuPhase events as coloured bars and a cursor line.
    ImGui::Text("cycle %llu / %llu", static_cast<unsigned long long>(replay.cursor()),
                static_cast<unsigned long long>(replay.result().total_cycles));
    ImGui::End();
}

void draw_warp_lanes(const Replay& replay) {
    ImGui::Begin("Warp lanes");
    // TODO: for each warp, find the latest WarpIssue event at or before the
    // cursor and draw one filled/empty box per lane from its mask.
    (void)replay;
    ImGui::End();
}

void draw_cache(const Replay& replay) {
    ImGui::Begin("Cache");
    // TODO: ImPlot line plot of cumulative hits and misses per cache up to
    // the cursor.
    (void)replay;
    ImGui::End();
}

void draw_stats(const Replay& replay) {
    ImGui::Begin("Stats");
    if (replay.loaded()) {
        const auto& r = replay.result();
        ImGui::Text("%s", r.passed ? "PASS" : "FAIL");
        ImGui::Text("cycles: %llu", static_cast<unsigned long long>(r.total_cycles));
        // TODO: L2 hit rate, lines/request, divergence efficiency.
    } else {
        ImGui::Text("No run loaded.");
    }
    ImGui::End();
}

}  // namespace gui
