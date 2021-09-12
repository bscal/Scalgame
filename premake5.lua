workspace "Game"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Game"
    kind "ConsoleApp"
    language "C++"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "../src/**.h",
        "../src/**.cpp"
    }

    includedirs
    {
        "../src",
        "../vendor/",
        "F:/dev-libs/vcpkg/vcpkg/installed/x64-windows/include"
    }

    libdirs
    {
        "../vendor/raylib/lib",
        "F:/dev-libs/vcpkg/vcpkg/installed/x64-windows/lib"
    }

    links
    {
        "raylib",
        "gamenetworkingsocket"
    }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"

    filter "configurations:Dist"
        optimize "On"