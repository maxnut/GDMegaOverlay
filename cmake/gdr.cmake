include(FetchContent)

FetchContent_Declare(
    gdr
    GIT_REPOSITORY https://github.com/maxnut/GDReplayFormat.git
    GIT_TAG        af2eafe478968835dde3e9e9837db27a72632d7c
    GIT_PROGRESS TRUE
)
message("Fetching gdr")
FetchContent_MakeAvailable(gdr)
