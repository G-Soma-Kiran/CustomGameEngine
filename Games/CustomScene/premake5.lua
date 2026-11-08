project "CustomScene"
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

project "CustomSceneApp"
kind "ConsoleApp"
targetname "CustomScene"
targetdir "../../bin/%{cfg.buildcfg}"
files { "main.cpp" }
links { "CustomScene", "EngineCore" }
UseSFML()