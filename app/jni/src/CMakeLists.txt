cmake_minimum_required(VERSION 3.6)

file(GLOB_RECURSE main_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

project(MY_APP)

find_library(SDL2 SDL2)

add_library(main SHARED)

target_sources(main PRIVATE ${main_sources})

target_link_libraries(main
	android
	log
	SDL2
	SDL2_image
	SDL2_gfx
	SDL2_mixer
	SDL2_ttf)


