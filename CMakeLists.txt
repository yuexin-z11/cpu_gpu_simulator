cmake_minimum_required(VERSION 3.16)
project(socsim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

option(SOCSIM_BUILD_TESTS "Build the component and integration checks" ON)
option(SOCSIM_BUILD_GUI   "Build the socsim-gui replay window (needs a display to run)" OFF)
option(SOCSIM_BUILD_CUDA  "Build the socsim-cuda trend check (needs the CUDA Toolkit)" OFF)
option(SOCSIM_FETCH_DEPS  "Download dependencies that are not found" ON)

# Look inside an activated conda environment (ece4122-6122) first.
if(DEFINED ENV{CONDA_PREFIX})
  list(APPEND CMAKE_PREFIX_PATH "$ENV{CONDA_PREFIX}")
endif()

# ------------------------------------------------------------ nlohmann/json -
find_package(nlohmann_json 3.11 CONFIG QUIET)
if(NOT TARGET nlohmann_json::nlohmann_json)
  if(NOT SOCSIM_FETCH_DEPS)
    message(FATAL_ERROR "nlohmann/json not found. Install it or enable SOCSIM_FETCH_DEPS.")
  endif()
  message(STATUS "nlohmann/json: not found - fetching 3.11.3")
  include(FetchContent)
  FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
  FetchContent_MakeAvailable(nlohmann_json)
else()
  message(STATUS "nlohmann/json: package config")
endif()

# ------------------------------------------------------------ Warnings ------
function(socsim_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  elseif(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  endif()
endfunction()

# ------------------------------------------------------------ Library -------
add_library(socsim_core
  src/config.cpp
  src/flat_memory.cpp
  src/cache.cpp
  src/dram.cpp
  src/memory_system.cpp
  src/elf_loader.cpp
  src/decoder.cpp
  src/executor.cpp
  src/cpu_core.cpp
  src/warp.cpp
  src/coalescer.cpp
  src/gpu_core.cpp
  src/stats.cpp
  src/event_log.cpp
  src/simulator.cpp
  src/sweep_runner.cpp)
target_include_directories(socsim_core PUBLIC include)
target_link_libraries(socsim_core PUBLIC nlohmann_json::nlohmann_json)
find_package(Threads REQUIRED)
target_link_libraries(socsim_core PUBLIC Threads::Threads)
socsim_warnings(socsim_core)

# ------------------------------------------------------------ CLI -----------
add_executable(socsim app/main.cpp)
target_link_libraries(socsim PRIVATE socsim_core)
socsim_warnings(socsim)

# ------------------------------------------------------------ Tests ---------
if(SOCSIM_BUILD_TESTS)
  enable_testing()
  add_executable(socsim_tests
    tests/test_main.cpp
    tests/test_decoder.cpp
    tests/test_cache.cpp
    tests/test_coalescer.cpp
    tests/test_end_to_end.cpp
    tests/test_determinism.cpp)
  target_link_libraries(socsim_tests PRIVATE socsim_core)
  target_compile_definitions(socsim_tests PRIVATE
    SOCSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  socsim_warnings(socsim_tests)
  add_test(NAME socsim_tests COMMAND socsim_tests)
endif()

# ------------------------------------------------------------ Optional ------
if(SOCSIM_BUILD_GUI)
  add_subdirectory(gui)
endif()
if(SOCSIM_BUILD_CUDA)
  add_subdirectory(cuda)
endif()
