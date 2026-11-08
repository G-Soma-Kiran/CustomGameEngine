project "OrbPop"
kind "StaticLib"
targetdir "../../lib/%{cfg.buildcfg}"
files
{
    "*.cpp",
    "*.hpp"
}
removefiles { "main.cpp" }
includedirs { "." }
links { "EngineCore" }

project "OrbPopApp"
kind "ConsoleApp"
targetname "OrbPop"
targetdir "../../bin/%{cfg.buildcfg}"
files { "main.cpp" }
links { "OrbPop", "EngineCore" }
UseSFML()