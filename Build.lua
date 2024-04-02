-- premake5.lua
workspace "Termporal"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Runtime"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

   OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

   include "Temporal_External.lua"
   include "runtime/Build-Runtime.lua"