# Core library
# Collect all source files
file(GLOB_RECURSE CORE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source/**.cpp")
file(GLOB_RECURSE CORE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/source/**.h")
file(GLOB_RECURSE UNIT_TESTS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Unit Tests/**.cpp")
file(GLOB_RECURSE UNIT_TESTS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Unit Tests/**.h")


# Core library
add_library(Core STATIC 
${CORE_SOURCE} 
${CORE_HEADERS} 
${UNIT_TESTS_SOURCE}
${UNIT_TESTS_HEADERS}
)

find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
message(STATUS "Vulkan found: ${Vulkan_INCLUDE_DIRS}")
else()
message(FATAL_ERROR "Vulkan not found")
endif()

include_directories(${Vulkan_INCLUDE_DIRS})
# Link GLFW and Vulkan to Runtime executable

target_link_libraries(Core PRIVATE glfw)
target_link_libraries(Core PRIVATE Vulkan::Vulkan)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_include_directories(Core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Unit Tests")
