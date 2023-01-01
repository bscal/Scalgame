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
        "src",
        "vendor",
        "vendor/raylib/src"
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

    filter "system:Unix"
        defines "SCAL_PLATFORM_LINUX"

    filter {}

    postbuildcommands
    {
        ("{COPYFILE} %{wks.location}bin/" .. outputdir .. "/%{prj.name}/Engine.dll "
            .. "%{wks.location}bin/" .. outputdir .. "/%{prj.name}/../Game/Engine.dll")
    }

