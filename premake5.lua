workspace "AutoUnzip"
architecture "x64"
    configurations { "Debug", "Release" }
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Core"
    location "%{prj.name}"
    kind "ConsoleApp"
    language "C++"
    targetname "AutoUnzip"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp"
,        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/src",
        "minizip/include"
    }

    libdirs "%{prj.name}/lib"

    links
    {
        "minizip",
        "opengl32",
        "gdi32",
        "winmm",
        "freetype",
        "shell32"
    }

    defines "SFML_STATIC"

    filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
        links
        {
            "zlibstatic-d",
            "sfml-system-s-d",
            "sfml-window-s-d",
            "sfml-graphics-s-d"
        }

    filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
        kind "WindowedApp"
        links
        {
            "zlibstatic",
            "sfml-system-s",
            "sfml-window-s",
            "sfml-graphics-s"
        }

project "minizip"
    location "%{prj.name}"
    kind "StaticLib"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp"
,        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/src"
    }

    filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"

    filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"

