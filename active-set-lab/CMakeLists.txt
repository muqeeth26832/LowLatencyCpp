cmake_minimum_required(VERSION 3.20)

project(ActiveSetBench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark REQUIRED)

function(add_active_set_benchmark target source)
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

add_active_set_benchmark(
    iterate
    benchmarks/iterate.cpp
)

add_active_set_benchmark(
    insert_only
    benchmarks/insert_only.cpp
)

add_active_set_benchmark(
    remove_only
    benchmarks/remove_only.cpp
)

add_active_set_benchmark(
    bulk_lifecycle
    benchmarks/bulk_lifecycle.cpp
)

add_active_set_benchmark(
    mixed_churn
    benchmarks/mixed_churn.cpp
)

add_active_set_benchmark(
    realistic
    benchmarks/realistic.cpp
)