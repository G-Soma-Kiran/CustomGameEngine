project "Chess"
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

project "ChessApp"
kind "ConsoleApp"
targetname "Chess"
targetdir "../../bin/%{cfg.buildcfg}"
files { "main.cpp" }
links { "Chess", "EngineCore" }
UseSFML()