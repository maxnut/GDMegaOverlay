cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(PROJECT GDMO)
project(${PROJECT})

file(GLOB_RECURSE SOURCE_FILES
  src/*.cpp
  src/*.h
  src/*.c
  src/GUI/*.cpp
  src/GUI/*.h

  libs/discord_game_sdk/cpp/*.cpp
)

add_library(${PROJECT} SHARED ${SOURCE_FILES})

if(MSVC)
  target_compile_options(${PROJECT} PRIVATE /W3)
endif()

# move to using .cmake files to get needed libs
target_include_directories(${PROJECT} PRIVATE
  libs/minhook/include
  libs/cocos-headers/cocos2dx
  libs/cocos-headers/cocos2dx/include
  libs/cocos-headers/cocos2dx/kazmath/include
  libs/cocos-headers/cocos2dx/platform/win32
  libs/cocos-headers/cocos2dx/platform/third_party/win32/OGLES
  libs/cocos-headers/extensions
  libs/imgui-hook
  libs/imgui-hook/imgui
  libs/discord_game_sdk/cpp
  libs/portaudio/include
  libs/libsndfile/include
)

add_subdirectory(libs/minhook)
add_subdirectory(libs/imgui-hook)
add_subdirectory(libs/cocos-headers)
add_subdirectory(libs/fmod-headers)

target_link_libraries(${PROJECT} minhook imgui-hook opengl32 cocos2d fmod)

target_link_libraries(${PROJECT}
  ${CMAKE_SOURCE_DIR}/libs/glew/glew32.lib
  ${CMAKE_SOURCE_DIR}/libs/cocos-headers/cocos2dx/robtop/glfw/glfw3.lib
  ${CMAKE_SOURCE_DIR}/libs/discord_game_sdk/lib/x86/discord_game_sdk.dll.lib
  ${CMAKE_SOURCE_DIR}/libs/portaudio/lib/portaudio_x86.lib
  ${CMAKE_SOURCE_DIR}/libs/libsndfile/lib/sndfile.lib
)

# Enable dev console if building in RelWithDebInfo 
target_compile_definitions(${PROJECT} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:DEV_CONSOLE>")
