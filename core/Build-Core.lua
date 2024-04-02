project "Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Build/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp",  "Source/**.impl.h" }

   includedirs
   {
      "Source",

      "../vendor/",
      "../vendor/glfw/include/",

      "%{IncludeDir.VulkanSDK}",
   }
 
    links
    {        
        "GLFW",   
        "%{Library.Vulkan}",
    }
   targetdir ("../Build/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Build/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

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