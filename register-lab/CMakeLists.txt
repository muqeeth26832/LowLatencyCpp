cmake_minimum_required(VERSION 3.20)

project(RegisterLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(benchmark REQUIRED)
find_package(Threads REQUIRED)

function(add_register_benchmark target source)
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
        Threads::Threads
    )
endfunction()

add_register_benchmark(
    register
    benchmarks/register.cpp
)