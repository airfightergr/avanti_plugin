cmake_minimum_required(VERSION 3.26)
project(avanti)

set(CMAKE_OSX_DEPLOYMENT_TARGET "12.00" CACHE STRING "Minimum macOS version" FORCE)
# We will enable this when we have a SDK supporting arm
# set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for mac OS X" FORCE)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for mac OS X" FORCE)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for mac OS X" FORCE)

# By default, use optimized release configuration.
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM400=1)

file(GLOB LIBACFUTILS_LIBS libacfutils/lin64/lib/*.a)
if(WIN32)
    list(APPEND LIBACFUTILS_LIBS crypt32 ws2_32 gdi32 dbghelp psapi winmm)
else()
    list(APPEND LIBACFUTILS_LIBS pthread)
endif()

include_directories("SDK/CHeaders/XPLM")
include_directories("SDK/CHeaders/Wrappers")
include_directories("SDK/CHeaders/Widgets")
include_directories("libacfutils/include")
include_directories("libacfutils/lin64/include")
include_directories("imgui")

# Specify library search locations.
if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/SDK/Libraries/Win")
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/SDK/Libraries/Mac")
elseif (UNIX)

endif ()


add_library(avanti SHARED src/avanti.c
        src/units_conv.c src/units_conv.h
        src/pfd.c src/pfd.h
        src/avanti.h
        src/file_io.h src/file_io.c
        src/avanti_gui.cpp src/avanti_gui.h
        src/avanti_gui.cpp
)

target_compile_options(avanti PUBLIC -Wall -Wextra )
#target_link_libraries(avanti PUBLIC ${LIBACFUTILS_LIBS} )
#target_link_libraries(avanti PUBLIC ${LIBACFUTILS_LIBS} ${OPENGL_LIBRARIES})

# Link library for dynamic loading of shared objects on UNIX systems.
if (UNIX)
    find_library(DL_LIBRARY dl)
    target_link_libraries(avanti PUBLIC ${LIBACFUTILS_LIBS} ${OPENGL_LIBRARIES} ${DL_LIBRARY} )
endif ()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
    add_definitions(-DAPL=1 -DIBM=0 -DLIN=0)
    set(ARCH "mac64")
elseif(WIN32)
    add_definitions(-DAPL=0 -DIBM=1 -DLIN=0)
    add_definitions(-D_WIN32_WINNT=0x0600)
    set(ARCH "mingw64")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}  -static-libgcc -static-libstdc++ -s")
else()
    add_definitions(-DAPL=0 -DIBM=0 -DLIN=1)
    set(ARCH "lin64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -s")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}  -static-libgcc -static-libstdc++ -s")
endif()

set_target_properties(avanti PROPERTIES PREFIX "")
set_target_properties(avanti PROPERTIES SUFFIX "")
set_target_properties(avanti PROPERTIES OUTPUT_NAME "avanti.xpl")

# build directly to the proper folder
set(PLUGIN_BUILD_DIR "${PROJECT_SOURCE_DIR}/../avanti")

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

set_target_properties(avanti PROPERTIES LINK_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -fno-stack-protector -fPIC -fvisibility=hidden")
