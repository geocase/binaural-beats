cmake_minimum_required(VERSION 3.1.0)
project(grid C)
set(EXE_NAME grid)

include_directories(vendor/glad/include/)
include_directories(vendor/stb/)

include_directories(vendor/freetype-2.10.4/include)
link_directories(vendor/freetype-2.10.4/win64/)

include_directories(vendor/glfw-3.3.4/include/)
link_directories(vendor/glfw-3.3.4/lib-vc2019/)

include_directories(vendor/openal-soft-1.21.1-bin/include)
link_directories(vendor/openal-soft-1.21.1-bin/libs/Win64)

include_directories(vendor/libsndfile/include)
link_directories(vendor/libsndfile/lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

include_directories(src)

set(SOURCES src/main.c src/gui.c)
add_executable(${EXE_NAME} ${SOURCES} )
set_property(TARGET ${EXE_NAME} PROPERTY C_STANDARD 11)

if(WIN32)
    target_compile_options(${EXE_NAME} PRIVATE)
    target_link_libraries(${EXE_NAME} OpenAL32 libsndfile-1 ${GTK3_LIBRARIES})
endif(WIN32)
if(UNIX)
    target_compile_options(${EXE_NAME} PRIVATE)
    target_link_libraries(${EXE_NAME} SDL2 SDL2_image dl pthread)
endif(UNIX)
