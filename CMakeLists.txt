cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-w)

set(PROJECT AModMenu)
project(${PROJECT})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp libs/discord_game_sdk/cpp/*.cpp)
add_library(${PROJECT} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT} PRIVATE
  libs/minhook/include
  libs/gd.h/include
  libs/gd.h/
  libs/cocos-headers/cocos2dx/
  libs/cocos-headers/cocos2dx/include
  libs/cocos-headers/cocos2dx/kazmath/include
  libs/cocos-headers/cocos2dx/platform/win32
  libs/cocos-headers/cocos2dx/platform/third_party/win32
  libs/cocos-headers/cocos2dx/platform/third_party/win32/OGLES
  libs/cocos-headers/extensions/
  libs/imgui-hook
  libs/imgui-hook/imgui
  libs/cocos-headers/cocos2dx/platform/third_party/win32/curl
  libs/fmod/inc
  libs/discord_game_sdk/cpp
)

add_subdirectory(libs/minhook)
add_subdirectory(libs/imgui-hook)

target_link_libraries(${PROJECT} minhook imgui-hook opengl32) # OpenGL32 for imgui

target_link_libraries(${PROJECT}
  ${CMAKE_SOURCE_DIR}/libs/cocos-headers/cocos2dx/libcocos2d.lib
  ${CMAKE_SOURCE_DIR}/libs/cocos-headers/extensions/libExtensions.lib
  ${CMAKE_SOURCE_DIR}/libs/fmod/lib/x86/fmod_vc.lib
  ${CMAKE_SOURCE_DIR}/libs/glew32.lib
  ${CMAKE_SOURCE_DIR}/libs/discord_game_sdk/lib/x86/discord_game_sdk.dll.lib
)
