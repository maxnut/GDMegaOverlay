include(FetchContent)

FetchContent_Declare(
    libsndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        e5ee50fbda1b9049a45fc65d06c34825feb4f237
    GIT_PROGRESS TRUE
)
message("Fetching libsndfile")
FetchContent_MakeAvailable(libsndfile)
