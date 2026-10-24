# socsim-cuda: runs the GPU comparison workloads on a real NVIDIA GPU.
# Separate from socsim_core; never changes simulator results.

enable_language(CUDA)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Build for the GPU in this machine unless an architecture list is given,
# e.g. -DCMAKE_CUDA_ARCHITECTURES=86 locally or 70;80 on PACE-ICE.
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES OR CMAKE_CUDA_ARCHITECTURES STREQUAL "")
  set(CMAKE_CUDA_ARCHITECTURES native)
endif()

add_executable(socsim-cuda trends.cu)
set_target_properties(socsim-cuda PROPERTIES CUDA_ARCHITECTURES "${CMAKE_CUDA_ARCHITECTURES}")
