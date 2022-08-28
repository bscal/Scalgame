project "Game"
    kind "ConsoleApp"
    language "C++"
    staticruntime "off"
    cppdialect "C++17"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.cpp",
        "src/**.h"
    }
    
    defines
    {
    }

    includedirs
    {
        "src",
        "vendor",
        "%{wks.location}/Engine/src",
        "%{wks.location}/Engine/vendor"
    }

    links
    {
        "Engine",
        "raylib",
        "winmm", "kernel32", "opengl32", "gdi32"
    }

    filter "configurations:Debug"
        defines "SCAL_DEBUG"
        runtime "Debug"
        symbols "on"
        staticruntime "off"

    filter "configurations:Release"
        defines "SCAL_RELEASE"
        runtime "Release"
        optimize "on"
        staticruntime "off"

    filter "system:Windows"
        defines "SCAL_PLATFORM_WINDOWS"
        systemversion "latest"

    filter "system:Unix"
        defines "SCAL_PLATFORM_LINUX"

