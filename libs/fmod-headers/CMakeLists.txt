cmake_minimum_required(VERSION 3.0.0)

project(fmod CXX)

add_library(fmod INTERFACE)

target_include_directories(fmod INTERFACE 
  inc/
)

target_link_libraries(fmod INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/lib/fmod_vc.lib
)
