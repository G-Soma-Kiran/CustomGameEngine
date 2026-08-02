workspace "GameDev"
architecture "x86_64"

configurations
{
    "Debug","Release"
}

project "Engine"
kind "ConsoleApp"
language "C++"
cppdialect "C++20"
warnings "Extra"
targetdir "bin/%{cfg.buildcfg}"
objdir "build/%{cfg.buildcfg}"
files
{
    "*.cpp",
    "*.hpp"
}
-- includedirs
-- {
--     "C:/msys64/ucrt64/include"
-- }
-- libdirs
-- {
--     "C:/msys64/ucrt64/lib"
-- }
links
{
    "sfml-graphics",
    "sfml-window",
    "sfml-system"
}
filter "configurations:Debug"
    symbols "On"
filter "configurations:Release"
    optimize "On"
filter {}