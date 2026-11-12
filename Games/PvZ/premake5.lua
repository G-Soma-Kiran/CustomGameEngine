project "PvZ"
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

project "PvZApp"
kind "ConsoleApp"
targetname "PvZ"
targetdir "../../bin/%{cfg.buildcfg}"
files { "main.cpp" }
links { "PvZ", "EngineCore" }
UseSFML()