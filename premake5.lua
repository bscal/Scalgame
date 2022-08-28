workspace "ScalGame"
    architecture "x64"
    staticruntime "off"

    configurations
    {
        "Debug",
        "Release"
    }

    startproject "Game"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

newoption
{
    trigger = "graphics",
    value = "OPENGL_VERSION",
    description = "version of OpenGL to build raylib against",
    allowed = {
	    { "opengl11", "OpenGL 1.1"},
	    { "opengl21", "OpenGL 2.1"},
	    { "opengl33", "OpenGL 3.3"},
	    { "opengl43", "OpenGL 4.3"}
    },
    default = "opengl43"
}

include "Engine/vendor/raylib_premake5.lua"
include "Engine"
include "Game"

newaction
{
    trigger = "clean",
    description = "Remove all binaries, int-binaries, vs files",
    execute = function()
        os.rmdir("./bin")
        print("Successfully removed binaries")
        os.rmdir("./bin-int")
        print("Successfully removed intermediate binaries")
        os.rmdir("./.vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Successfully removed vs project files")
        print("Done!")
    end
}
