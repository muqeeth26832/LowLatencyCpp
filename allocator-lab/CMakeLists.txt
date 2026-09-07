cmake_minimum_required(VERSION 3.20)

project(AllocatorBench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark REQUIRED)

function(add_allocator_benchmark target source)
    add_executable(${target} ${source})

    target_include_directories(
        ${target}
        PRIVATE
        include
    )

    target_link_libraries(
        ${target}
        PRIVATE
        benchmark::benchmark
    )
endfunction()

add_allocator_benchmark(
    allocation_only
    benchmarks/allocation_only.cpp
)

add_allocator_benchmark(
    alloc_free
    benchmarks/alloc_free.cpp
)

add_allocator_benchmark(
    burst_cleanup
    benchmarks/burst_cleanup.cpp
)

add_allocator_benchmark(
    random_lifetime
    benchmarks/random_lifetime.cpp
)

add_allocator_benchmark(
    mixed_sizes
    benchmarks/mixed_sizes.cpp
)
