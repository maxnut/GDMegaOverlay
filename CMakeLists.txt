cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(GDMegaOverlay VERSION 1.0.0)

file(GLOB SOURCES
src/*.cpp
src/*.h
src/*.c
src/GUI/*.cpp
src/GUI/*.h
src/Hacks/*.cpp
src/JsonHacks/*.cpp
src/Macrobot/*.cpp
src/DiscordRPCManager/*.cpp

libs/discord_game_sdk/cpp/*.cpp
)

# Set up the mod binary
add_library(${PROJECT_NAME} SHARED
${SOURCES}
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
  libs/discord_game_sdk/cpp
  libs/portaudio/include
  libs/libsndfile/include
  libs/gdr
  src/
)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

set(IMGUI_VERSION "v1.89.2")
CPMAddPackage("gh:matcool/gd-imgui-cocos#4eb5777")

target_link_libraries(${PROJECT_NAME} imgui-cocos)

target_link_libraries(${PROJECT_NAME}
  ${CMAKE_SOURCE_DIR}/libs/discord_game_sdk/lib/x86/discord_game_sdk.dll.lib
  ${CMAKE_SOURCE_DIR}/libs/portaudio/lib/portaudio_x86.lib
  ${CMAKE_SOURCE_DIR}/libs/libsndfile/lib/sndfile.lib
)

# Set up dependencies, resources, link Geode
setup_geode_mod(${PROJECT_NAME})
