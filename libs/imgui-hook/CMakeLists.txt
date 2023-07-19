cmake_minimum_required(VERSION 3.0)
project(imgui-hook)
add_library(imgui-hook STATIC
    src/imgui-hook.cpp
    src/imgui/imgui.cpp
	src/imgui/imgui_widgets.cpp
	src/imgui/imgui_tables.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/backends/imgui_impl_win32.cpp
    src/imgui/backends/imgui_impl_opengl3.cpp
    src/imgui/misc/cpp/imgui_stdlib.cpp
)
target_link_libraries(imgui-hook cocos2d)
target_include_directories(imgui-hook PUBLIC include/imgui include)
set_property(TARGET imgui-hook PROPERTY CXX_STANDARD 17)