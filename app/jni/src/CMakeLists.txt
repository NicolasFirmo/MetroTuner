cmake_minimum_required(VERSION 3.6)

file(GLOB_RECURSE main_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)
list(REMOVE_ITEM ${folderName}_sources pch.h)

project(MY_APP)

find_library(SDL2 SDL2)

add_library(main SHARED)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(main PRIVATE ${main_sources})

target_precompile_headers(main PRIVATE pch.h)

target_link_libraries(main
	android
	log
	SDL2
	SDL2_image
	SDL2_gfx
	SDL2_mixer
	SDL2_ttf)


