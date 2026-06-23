cmake_minimum_required(VERSION 3.15)
project(ToySTA VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ToySTA
    src/main.cpp
    src/sta/timing_graph.cpp
    src/sta/timing_engine.cpp
    src/sta/circuit_reader.cpp
)

target_include_directories(ToySTA PRIVATE src)
