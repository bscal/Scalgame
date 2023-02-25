project "Engine"
    kind "SharedLib"
    language "C++"
    staticruntime "off"
    cppdialect "C++17"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.cpp",
        "src/**.h",
        "src/**.hpp",
        "src/**.c"
    }

    defines
    {
        "SCAL_BUILD_DLL"
    }

    includedirs
    {
        "src/",
        "vendor/raylib/src/",
        "vendor/",
    }

    links
    {
        "raylib", "winmm", "kernel32", "opengl32", "gdi32"
    }

    filter "configurations:Debug"
        defines "SCAL_DEBUG"
        buildoptions "/MDd"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "SCAL_RELEASE"
        buildoptions "/MD"
        runtime "Release"
        optimize "on"

    filter "system:Windows"
        defines "SCAL_PLATFORM_WINDOWS"
        systemversion "latest"
        buildoptions
        { 
            "-std=c++17", "-Wc++17-compat", "-Weverything", "-Wno-c++98-compat-pedantic",
            "-Wno-old-style-cast", "-Wno-extra-semi-stmt"
        }

    filter "system:Unix"
        defines "SCAL_PLATFORM_LINUX"

    filter {}

    postbuildcommands
    {
        ("{COPYFILE} %{wks.location}bin/" .. outputdir .. "/%{prj.name}/Engine.dll "
            .. "%{wks.location}bin/" .. outputdir .. "/%{prj.name}/../Game/Engine.dll")
    }

