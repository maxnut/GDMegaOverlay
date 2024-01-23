find_package(Git)
if(Git_FOUND)
    message("Git found: ${GIT_EXECUTABLE}")
endif()
