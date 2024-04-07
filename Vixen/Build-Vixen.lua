project "Vixen"
   kind "SharedLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Build/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp",  "Source/**.impl.h" }

   includedirs
   {
      "Source",
      "../core/source",

      "../submodules/",
      "../submodules/glfw/include/",
      "../submodules/gtest/googletest/include/",
      
      "%{IncludeDir.VulkanSDK}",
    }
    
    links
    { 
        "../submodules/gtest/lib/%{cfg.buildcfg}/gtest.lib",
        "Core",       
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