cmake_minimum_required(VERSION 3.0)

add_library(imgui-hook STATIC
    imgui-hook.cpp
    imgui/imgui.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_tables.cpp
    imgui/imgui_draw.cpp
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
)
set_property(TARGET imgui-hook PROPERTY CXX_STANDARD 17)
target_include_directories(imgui-hook PRIVATE imgui
  ../cocos-headers/libs/cocos2dx/include
  ../cocos-headers/libs/cocos2dx/include/kazmath/include
  ../cocos-headers/libs/cocos2dx/include/platform/win32
  ../cocos-headers/libs/cocos2dx/include/platform/third_party/win32
  ../cocos-headers/libs/cocos2dx/include/platform/third_party/win32/OGLES
  ../cocos-headers/libs/extensions/include
)