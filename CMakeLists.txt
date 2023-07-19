cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-w)

set(PROJECT GDMenu)
project(${PROJECT})

add_subdirectory(libs/minhook)
add_subdirectory(libs/imgui-hook)
add_subdirectory(libs/cocos-headers)
add_subdirectory(libs/fmod-headers)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp libs/discord_game_sdk/cpp/*.cpp)
add_library(${PROJECT} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT} minhook cocos2d imgui-hook opengl32 fmod) # OpenGL32 for imgui

target_include_directories(${PROJECT} PRIVATE
  libs/minhook/include
  libs/gd.h/include
  libs/gd.h/
  libs/cocos-headers/libs/cocos2dx/include/platform/third_party/win32/curl
  libs/discord_game_sdk/cpp
)

target_link_libraries(${PROJECT}
  ${CMAKE_SOURCE_DIR}/libs/glew/glew32.lib
  ${CMAKE_SOURCE_DIR}/libs/discord_game_sdk/lib/x86/discord_game_sdk.dll.lib
)
