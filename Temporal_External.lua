-- Externals

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["SPIRV"] = "%{VULKAN_SDK}/glslang/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["GLFW"] = "glfw/"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"


Dependencies = {}
Dependencies["GLFW"] = "glfw"


group "Core"
    include "Core/Build-Core.lua"
group ""

group "Test Framework"
    include "Vixen/Build-Vixen.lua"
group ""

group "Submodules"
    include "submodules/glfw"
    -- include "submodules/volk"
group ""