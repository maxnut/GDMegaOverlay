include(FetchContent)

FetchContent_Declare(
    portable-file-dialogs
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
    GIT_TAG        7f852d88a480020d7f91957cbcefe514fc95000c
    GIT_PROGRESS TRUE
)
message("Fetching portable-file-dialogs")
FetchContent_MakeAvailable(portable-file-dialogs)
