include(FetchContent)

# Disable internal tests
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library.")

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark
        GIT_TAG main
        )
FetchContent_MakeAvailable(googlebenchmark)
