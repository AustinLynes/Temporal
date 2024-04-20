file(GLOB RT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")
file(GLOB RT_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")

add_executable(Runtime
    ${RT_SOURCES}
    ${RT_HEADERS}
    # Add more source files as needed
)

target_include_directories(Runtime PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/source  # runtime source directory
    "../Core/source"                    # core source directory
)

# vulkan needs to be explicitly found. 
# if no sdk is found on the host machine.
# the program cannot run.
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "Vulkan found: ${Vulkan_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Vulkan not found")
endif()


#include vulkan dirs
include_directories(${Vulkan_INCLUDE_DIRS})

# Link Core library to Runtime executable
target_link_libraries(Runtime PRIVATE Core)


# Link GLFW and Vulkan to Runtime executable
target_link_libraries(Runtime PRIVATE glfw)
target_link_libraries(Runtime PRIVATE Vulkan::Vulkan)
