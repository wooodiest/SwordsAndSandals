workspace "SwordsAndSandals"
    startproject "SwordsAndSandals"
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    architecture "x64"
    configurations 
    { 
        "Debug", 
        "Release" 
    }
    flags
    { 
        "MultiProcessorCompile" 
    }

project "SwordsAndSandals"
    kind "ConsoleApp"
    language "C"
    location "SwordsAndSandals"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{wks.location}/SwordsAndSandals/src/**.h",
		"%{wks.location}/SwordsAndSandals/src/**.c",
	}

    includedirs
    {
        "%{wks.location}/SwordsAndSandals/src",
    }

    filter "system:windows"
        defines "SAS_WINDOWS"
        systemversion "latest"

    filter "configurations:Debug"
	    defines "SAS_DEBUG"
	    runtime "Debug"
	    symbols "On"

	filter "configurations:Release"
	    defines "SAS_RELEASE"
	    runtime "Release"
	    optimize "on"
	    symbols "Off"

