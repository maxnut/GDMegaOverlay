include(FetchContent)

set(JSON_MultipleHeaders OFF)

# deprecated, move to nlohmann-json releases
FetchContent_Declare(
    nhjson
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
    GIT_TAG        326308d7512d2168ae00199aec4dd0f714526e89
    GIT_PROGRESS TRUE
)
message("Fetching nlohmann-json")
FetchContent_MakeAvailable(nhjson)
