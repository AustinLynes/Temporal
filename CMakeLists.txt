cmake_minimum_required(VERSION 3.10)
project(Temporal)

# Set the C++ standard to the latest available
set(CMAKE_CXX_STANDARD 20) # or 20 if your compiler supports it


# Include subdirectories
add_subdirectory(core)
add_subdirectory(runtime)
add_subdirectory(submodules/GLFW)

