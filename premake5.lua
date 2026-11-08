Games = {}
for _, d in ipairs(os.matchdirs("Games/*")) do
    if os.isfile(d .. "/premake5.lua") then
        table.insert(Games, path.getname(d))
    end
end

function UseSFML()
    links
    {
        "sfml-graphics",
        "sfml-window",
        "sfml-system"
    }
end

workspace "GameDev"
architecture "x86_64"
configurations { "Debug", "Release" }
startproject "Engine"

language "C++"
cppdialect "C++20"
warnings "Extra"
includedirs { "Include" }
objdir "build/%{cfg.buildcfg}/%{prj.name}"

filter "configurations:Debug"
    symbols "On"
filter "configurations:Release"
    optimize "On"
filter {}

project "EngineCore"
kind "StaticLib"
targetdir "lib/%{cfg.buildcfg}"
files
{
    "Src/**.cpp",
    "Include/**.hpp"
}

for _, g in ipairs(Games) do
    include("Games/" .. g)
end

project "Engine"
kind "ConsoleApp"
targetdir "bin/%{cfg.buildcfg}"
files { "main.cpp" }
links(Games)
links { "EngineCore" }
UseSFML()