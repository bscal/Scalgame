project "raylib"
    kind "SharedLib"
    staticruntime "off"
    language "C"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "raylib/src/*.c",
        "raylib/src/*.h"
    }

    includedirs
    {
        "raylib/src",
        "raylib/src/external",
        "raylib/src/external/glfw/include"
    }

    defines{"PLATFORM_DESKTOP", "BUILD_LIBTYPE_SHARED"}
    
    filter "system:windows"
        defines{"_WIN32"}
        links {"winmm", "kernel32", "opengl32", "gdi32"}

    filter "system:linux"
        links {"pthread", "GL", "m", "dl", "rt", "X11"}

    filter {"options:graphics=opengl43"}
        defines{"GRAPHICS_API_OPENGL_43"}

    filter {"options:graphics=opengl33"}
        defines{"GRAPHICS_API_OPENGL_33"}

    filter {"options:graphics=opengl21"}
        defines{"GRAPHICS_API_OPENGL_21"}

    filter {"options:graphics=opengl11"}
        defines{"GRAPHICS_API_OPENGL_11"}

    filter {"system:macosx"}
        disablewarnings {"deprecated-declarations"}

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"
        staticruntime "off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"
        staticruntime "off"

    filter { "platforms:x64" }
        architecture "x86_64"

    filter {}

    postbuildcommands
    {
        ("{COPYDIR} %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/raylib.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/../Engine/");
        ("{COPYDIR} %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/raylib.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/../Game/");
    }