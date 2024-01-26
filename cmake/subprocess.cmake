include(FetchContent)

FetchContent_Declare(
    subprocess
    GIT_REPOSITORY https://github.com/SpaghettDev/subprocess.git
    GIT_TAG        7a641a92eb73e8f39be4d0402efc2c46d5bd1b47
    GIT_PROGRESS TRUE
)
message("Fetching subprocess")
FetchContent_MakeAvailable(subprocess)
