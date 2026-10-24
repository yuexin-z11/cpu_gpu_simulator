# socsim-gui: replay window. Same dependency approach as ECE 6122 Lab 3:
# search the active conda environment, fetch what is missing, and load
# OpenGL through GLFW instead of linking libGL on Linux.

include(FetchContent)

# ---------------------------------------------------------------- GLFW -----
set(SOCSIM_GLFW_TARGET "")
find_package(glfw3 CONFIG QUIET)
if(TARGET glfw)
  set(SOCSIM_GLFW_TARGET glfw)
  message(STATUS "GLFW: package config")
else()
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(GLFW_PC QUIET IMPORTED_TARGET glfw3)
    if(TARGET PkgConfig::GLFW_PC)
      set(SOCSIM_GLFW_TARGET PkgConfig::GLFW_PC)
      message(STATUS "GLFW: pkg-config")
    endif()
  endif()
endif()
if(NOT SOCSIM_GLFW_TARGET)
  find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)
  find_library(GLFW_LIBRARY NAMES glfw glfw3)
  if(GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
    add_library(socsim_glfw INTERFACE)
    target_include_directories(socsim_glfw INTERFACE "${GLFW_INCLUDE_DIR}")
    target_link_libraries(socsim_glfw INTERFACE "${GLFW_LIBRARY}")
    set(SOCSIM_GLFW_TARGET socsim_glfw)
    message(STATUS "GLFW: header/library search (${GLFW_LIBRARY})")
  endif()
endif()
if(NOT SOCSIM_GLFW_TARGET AND SOCSIM_FETCH_DEPS)
  message(STATUS "GLFW: not found - fetching GLFW 3.4 (Linux needs X11 dev headers)")
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4)
  FetchContent_MakeAvailable(glfw)
  set(SOCSIM_GLFW_TARGET glfw)
endif()
if(NOT SOCSIM_GLFW_TARGET)
  message(FATAL_ERROR "GLFW not found. Activate the ece4122-6122 conda env or enable SOCSIM_FETCH_DEPS.")
endif()

# ------------------------------------------------------ Dear ImGui, ImPlot -
# Neither ships a CMake build, so their sources are compiled here.
# Release archives download much faster than a full git clone.
FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz)
FetchContent_Declare(implot
  URL https://github.com/epezent/implot/archive/refs/tags/v0.16.tar.gz)
FetchContent_MakeAvailable(imgui implot)

add_library(socsim_imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp)
target_include_directories(socsim_imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${implot_SOURCE_DIR})
# imgui_impl_opengl3 loads GL functions itself, so libGL is not linked on
# Linux (avoids the conda/system libGL conflict on PACE-ICE).
target_compile_definitions(socsim_imgui PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(socsim_imgui PUBLIC ${SOCSIM_GLFW_TARGET})
if(APPLE OR WIN32)
  find_package(OpenGL REQUIRED)
  target_link_libraries(socsim_imgui PUBLIC OpenGL::GL)
endif()

# ---------------------------------------------------------------- Target ---
add_executable(socsim-gui
  main.cpp
  replay.cpp
  panels.cpp)
target_include_directories(socsim-gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(socsim-gui PRIVATE socsim_core socsim_imgui)
target_compile_definitions(socsim-gui PRIVATE SOCSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
socsim_warnings(socsim-gui)
