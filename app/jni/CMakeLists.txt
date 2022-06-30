cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MetroTuner)

# armeabi-v7a requires cpufeatures library
include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()

include_directories(
        vendor/SDL2/include
        # SDL_internal.h is located in src required by SDL_android_main.c bridge
        vendor/SDL2/src
        vendor/SDL2_image/include
        vendor/SDL2_gfx/include
        vendor/SDL2_mixer/include
        vendor/SDL2_ttf/include
)

add_subdirectory(vendor/SDL2)
add_subdirectory(vendor/SDL2_image)
add_subdirectory(vendor/SDL2_gfx)
add_subdirectory(vendor/SDL2_mixer)
add_subdirectory(vendor/SDL2_ttf)

add_subdirectory(src)