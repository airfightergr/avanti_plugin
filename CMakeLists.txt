cmake_minimum_required(VERSION 3.26)
project(avanti VERSION 1.0 LANGUAGES C)


# X-Plane supports OS X 10.10+, so this should ensure our plugin can run on all supported versions.
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum macOS version" FORCE)
# This should also support arm64
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for mac OS X" FORCE)

# By default, use optimized release configuration.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

# language(s) standard
set(CMAKE_C_STANDARD 11)

file(GLOB LIBACFUTILS_LIBS libacfutils-redist/lin64/lib/*.a)
if(WIN32)
    list(APPEND LIBACFUTILS_LIBS crypt32 ws2_32 gdi32 dbghelp psapi winmm)
else()
    list(APPEND LIBACFUTILS_LIBS pthread)
endif()

include_directories(
        "${CMAKE_SOURCE_DIR}/SDK/CHeaders/XPLM"
        "${CMAKE_SOURCE_DIR}/SDK/CHeaders/Wrappers"
        "${CMAKE_SOURCE_DIR}/SDK/CHeaders/Widgets"
        "${CMAKE_SOURCE_DIR}/libacfutils-redist/include"
        "${CMAKE_SOURCE_DIR}/libacfutils-redist/lin64/include"
)

# Specify library search locations.
if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win")
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac")
elseif (UNIX)

endif ()

# Enable all X-Plane SDK APIs up to the newest version.
add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM302=1 -DXPLM303=1 -DXPLM400=1)
# Define platform macros.
add_definitions(-DAPL=$<BOOL:${APPLE}> -DIBM=$<BOOL:${WIN32}> -DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)

# Force-enable exception support. This is most likely redundant, although for C
# code the default is the opposite. Since we are mixing C++ and C libraries,
# safer to set it on?
add_compile_options(-fexceptions)
# On UNIX systems this makes symbols non-exported by default. On Windows this
# option is simply ignored, since symbol visibility works differently there.
add_compile_options(-fvisibility=hidden)

# Enable stricter warnings and then disable some we are not interested in.
add_compile_options(-Wall -Wpedantic -Wshadow -Wfloat-equal -Wextra)
add_compile_options(-Wno-unused)

# Always use position-independent code and highest optimization level (FPS!).
add_compile_options(-O3 -fPIC)

# define our plugin target
add_library(avanti SHARED
        src/avanti.c src/avanti.h
        src/units_conv.c src/units_conv.h
        src/pfd.c src/pfd.h
        src/file_io.h src/file_io.c
        #        src/commands.c src/commands.h
)

target_compile_features(avanti PUBLIC c_std_11)

#target_compile_options(avanti PUBLIC -Wall -Wextra )
#target_link_libraries(avanti PUBLIC ${LIBACFUTILS_LIBS} ${OPENGL_LIBRARIES})

if (APPLE)
    # X-Plane 12 supports OS X 11.3+, so this should ensure FlyWithLua can run on
    # all supported versions.
    target_compile_options(plugin PUBLIC -mmacosx-version-min=11.3)
    target_link_libraries(plugin -mmacosx-version-min=11.3)
endif ()

# Link OpenGL and OpenAL related libraries.
find_package(OpenGL REQUIRED)  # apt install freeglut3-dev
find_library(OPENAL_LIBRARY NAMES openal OpenAL OpenAL32)  # apt install libopenal-dev
find_library(GLUT_LIBRARY NAMES glut GLUT glut64)  # apt install freeglut3-dev
# target_link_libraries(plugin ${OPENGL_LIBRARIES} ${OPENAL_LIBRARY} ${GLUT_LIBRARY})


# Link X-Plane plugin system libraries. They are only provided for OS X and Windows.
if (WIN32 OR APPLE)
    # if (WIN32)
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib)
    find_library(XPWIDGETS_LIBRARY NAMES XPWidgets XPWidgets_64.lib)
    target_link_libraries(avanti ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY})
endif ()

# Link library for dynamic loading of shared objects on UNIX systems.
if (UNIX)
    find_library(DL_LIBRARY dl)
    target_link_libraries(avanti ${DL_LIBRARY})
endif ()

# Link OS X core system libraries.
if (APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(GLUT_LIBRARY GLUT)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(Cocoa_LIBRARY Cocoa)

    target_link_libraries(avanti ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY} ${GLUT_LIBRARY})
    target_link_libraries(avanti ${OpenGL_LIBRARY} ${Cocoa_LIBRARY})
endif ()

if (WIN32)
    # Unlike OS X and Linux we build standard libraries statically since X-Plane does not provide them.
    target_link_libraries(avanti -static-libgcc -static-libstdc++ opengl32)
    # MSYS2 for some reason always links against winpthread, so we have to work around it by statically linking the
    # entire plugin. This allows building the plugin nativaly on Windows.
    target_link_libraries(avanti -static)
elseif (APPLE)
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(avanti "-exported_symbols_list ${CMAKE_SOURCE_DIR}/exports.sym_mac")
    target_link_libraries(avanti "-framework XPLM -framework XPWidgets -ldl")
elseif (UNIX)
    # Specify additional runtime search laths for dynamically-linked libraries.
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(avanti -Wl,--version-script -Wl,"${CMAKE_SOURCE_DIR}/src/exports.txt")
    target_link_libraries(avanti ${LIBACFUTILS_LIBS})
endif ()

set(CMAKE_BUILD_TYPE RelWithDebInfo)

set_target_properties(avanti PROPERTIES PREFIX "")
set_target_properties(avanti PROPERTIES SUFFIX "")
set_target_properties(avanti PROPERTIES OUTPUT_NAME "avanti.xpl")

# build directly to the proper folder
set(PLUGIN_BUILD_DIR "${CMAKE_SOURCE_DIR}/../avanti")

if(WIN32)
    set_target_properties(avanti PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/win_x64")
    set_target_properties(avanti PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/win_x64")
elseif(APPLE)
    set_target_properties(avanti PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/mac_x64")
    set_target_properties(avanti PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/mac_x64")
else()
    set_target_properties(avanti PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/lin_x64")
    set_target_properties(avanti PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_BUILD_DIR}/lin_x64")
endif()

#set_target_properties(avanti PROPERTIES LINK_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector -fPIC -fvisibility=hidden")
