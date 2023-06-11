cmake_minimum_required(VERSION 3.5)

message(STATUS "Queue benchmarks")

include(${CMAKE_SOURCE_DIR}/cmake/benchmark.cmake)

file(GLOB BENCH_SOURCES "${CMAKE_SOURCE_DIR}/benchmarks/*.cpp")
message("BENCH_SOURCES: ${BENCH_SOURCES}")
foreach(BENCH_SOURCE ${BENCH_SOURCES})
    get_filename_component(BENCH_NAME ${BENCH_SOURCE} NAME_WE)
    set(BENCH_TARGET "queue_benchmark_${BENCH_NAME}")

    message(STATUS "Queue benchmark target '${BENCH_TARGET}'")

    add_executable(${BENCH_TARGET} ${BENCH_SOURCE})
    target_link_libraries(${BENCH_TARGET} PRIVATE benchmark::benchmark)
    target_include_directories(${BENCH_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/src)
endforeach()
