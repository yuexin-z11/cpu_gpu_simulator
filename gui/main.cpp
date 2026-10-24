// socsim-gui: runs a program with event recording and replays it.

#include <cstdio>
#include <cstring>
#include <exception>
#include <string>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "panels.hpp"
#include "replay.hpp"
#include "socsim/config.hpp"

int main(int argc, char** argv) {
    gui::UiState ui;
    std::string config_path = SOCSIM_SOURCE_DIR "/configs/default.json";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--program") == 0) ui.program = argv[++i];
        else if (std::strcmp(argv[i], "--config") == 0) config_path = argv[++i];
    }
    try {
        ui.config = socsim::load_config(config_path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }

    if (!glfwInit()) {
        std::fprintf(stderr, "error: glfwInit failed (is DISPLAY set?)\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 800, "SoCSim", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "error: could not create an OpenGL 3.3 window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    gui::Replay replay;
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        const double now = glfwGetTime();
        if (replay.playing && replay.loaded()) {
            // TODO: carry the fractional remainder so slow speeds still advance.
            replay.step(static_cast<socsim::Cycle>((now - last) * replay.cycles_per_second));
        }
        last = now;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        gui::draw_controls(ui, replay);
        gui::draw_timeline(replay);
        gui::draw_warp_lanes(replay);
        gui::draw_cache(replay);
        gui::draw_stats(replay);

        ImGui::Render();
        // Clearing is left to ImGui's background; no direct GL calls here.
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
