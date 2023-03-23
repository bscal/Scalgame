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
        "src/**.c",
        "vendor/rpmalloc/rpmalloc/*.c",
    }

    defines
    {
        "SCAL_BUILD_DLL"
    }

    includedirs
    {
        "src/",
        "vendor/",
        "vendor/raylib/src",
        "vendor/rpmalloc/rpmalloc"
    }

    links
    {
        "raylib"
    }

    filter "toolset:msc-ClangCL"
        buildoptions
        { 
            "-Wc++17-compat", "-Weverything", "-Wno-c++98-compat-pedantic",
            "-Wno-old-style-cast", "-Wno-extra-semi-stmt"
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
        defines { "PLATFORM_DESKTOP", "SCAL_PLATFORM_WINDOWS", "_WIN32" }
        systemversion "latest"

    filter "system:Unix"
        defines { "PLATFORM_DESKTOP", "SCAL_PLATFORM_LINUX" }

    filter {"options:graphics=opengl43"}
        defines{"GRAPHICS_API_OPENGL_43"}

    filter {"options:graphics=opengl33"}
        defines{"GRAPHICS_API_OPENGL_33"}

    filter {"options:graphics=opengl21"}
        defines{"GRAPHICS_API_OPENGL_21"}

    filter {"options:graphics=opengl11"}
        defines{"GRAPHICS_API_OPENGL_11"}

    filter {}

    postbuildcommands
    {
        ("{COPYFILE} %{wks.location}bin/" .. outputdir .. "/%{prj.name}/Engine.dll "
            .. "%{wks.location}bin/" .. outputdir .. "/%{prj.name}/../Game/Engine.dll")
    }

