workspace "Proyecto"
    architecture "x86"
    configurations { "Debug", "Release" }
    
project "Proyecto"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "bin-int/%{cfg.buildcfg}"

    -- Archivos del proyecto
    files {
        "Origen/**.cpp",
        "Origen/**.c",
        "Encabezado/**.h",
        "Textures/**.*",
        "Shaders/**.vert",
        "Shaders/**.frag",
        "Sonidos/**.*"
    }

    -- Carpetas de encabezados (.h)
    includedirs {
        "Encabezado",
        "Librerias/GLFW/include",
        "Librerias/GLAD/include",
        "Librerias/glm",
        "Librerias/Assimp/include",
        "Librerias/stb_image",
        "Librerias/miniaudio"
    }

    -- Carpetas donde están las .lib
    libdirs {
        "Librerias/GLFW/lib-vc2022",
        "Librerias/Assimp/lib/Win32"
    }

    -- Librerías necesarias
    links {
        "glfw3.lib",
        "opengl32.lib",
        "assimp-vc143-mt.lib"
    }

    -- Definiciones preprocesador
    defines {
        "GLFW_INCLUDE_NONE"
    }

    -- Configuración Debug
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

    -- Configuración Release
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"