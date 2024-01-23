include(FetchContent)

FetchContent_Declare(
    discord-rpc
    GIT_REPOSITORY https://github.com/discord/discord-rpc.git
    GIT_TAG        963aa9f3e5ce81a4682c6ca3d136cddda614db33
    GIT_PROGRESS TRUE
)
message("Fetching discord-rpc")
FetchContent_MakeAvailable(discord-rpc)
