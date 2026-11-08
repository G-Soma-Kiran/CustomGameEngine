project "InputTest"
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

project "InputTestApp"
kind "ConsoleApp"
targetname "InputTest"
targetdir "../../bin/%{cfg.buildcfg}"
files { "main.cpp" }
links { "InputTest", "EngineCore" }
UseSFML()