include(FetchContent)

FetchContent_Declare(
    gdr
    GIT_REPOSITORY https://github.com/maxnut/GDReplayFormat.git
    GIT_TAG        0b803b44a3769a9c450c113ef1aa028f2c960127
    GIT_PROGRESS TRUE
)
message("Fetching gdr")
FetchContent_MakeAvailable(gdr)
