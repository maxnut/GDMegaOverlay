include(FetchContent)

FetchContent_Declare(
    gdr
    GIT_REPOSITORY https://github.com/maxnut/GDReplayFormat.git
    GIT_TAG        4950cc287aaf12d4a92a6bd967aa664d4846d760
    GIT_PROGRESS TRUE
)
message("Fetching gdr")
FetchContent_MakeAvailable(gdr)
