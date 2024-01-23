include(FetchContent)

FetchContent_Declare(
    gdr
    GIT_REPOSITORY https://github.com/SpaghettDev/gdr.git
    GIT_TAG        c1c0ea3e5dc8c0584eff34b54fe4b6e632a3d050
    GIT_PROGRESS TRUE
)
message("Fetching gdr")
FetchContent_MakeAvailable(gdr)
