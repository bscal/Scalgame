workspace "Game"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

-- To the vcpkg directory containing your includes and libs folders. Didnt think premake's architecture would be the same as vcpkg's
vcpkg_installed_dir = "F:/dev-libs/vcpkg/vcpkg/installed/x64-windows"
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Game"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "vendor/raylib/include",
        vcpkg_installed_dir .. "/include"
    }

    libdirs
    {
        "vendor/raylib/lib",
        vcpkg_installed_dir .. "/lib"
    }

    links
    {
        "raylib",
        "GameNetworkingSockets"
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