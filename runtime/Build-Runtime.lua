project "Runtime"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Build/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp",  }

   includedirs
   {
      "Source",

      "../Core/Source",
      "../Core/Unit Tests",

      "../submodules/",
      "../submodules/glfw/include/",

      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.SPIRV}",
      
   }
 
    links
    {        
        "Core",
        "GLFW",
        "%{Library.Vulkan}"
    }


   targetdir ("../Build/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Build/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"