include(FetchContent)

FetchContent_Declare(
    gdr
    GIT_REPOSITORY https://github.com/maxnut/GDReplayFormat.git
    GIT_TAG        abb26e1f820883b0fa59d84674047ecd3404d124
    GIT_PROGRESS TRUE
)
message("Fetching gdr")
FetchContent_MakeAvailable(gdr)
